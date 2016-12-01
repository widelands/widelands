/*
 * Copyright 2016 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef WL_GRAPHIC_GL_STREAMING_BUFFER_H
#define WL_GRAPHIC_GL_STREAMING_BUFFER_H

#include <cassert>

#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/gl/system_headers.h"

namespace Gl {

// Wrapper around an OpenGL buffer object that is intended for streaming use.
//
// Requires GL_ARB_direct_state_access.
template <typename T> class StreamingBuffer {
public:
	class Inserter {
	public:
		Inserter(Inserter&& o)
		  : buffer_(o.buffer_), map_(o.map_), count_(o.count_), max_(o.max_) {
			o.buffer_ = nullptr;
			o.map_ = nullptr;
		}

		~Inserter() {
			assert(!buffer_ || buffer_->inserting_);
			if (buffer_)
				buffer_->inserting_ = false;

			if (map_) {
				buffer_->bind();
				glUnmapBuffer(buffer_->target());
			}
		}

		// The number of elements that have already been inserted.
		size_t count() const {
			return count_;
		}

		// The maximum number of elements that can be inserted.
		size_t max() const {
			return max_;
		}

		// Finish inserting. Return the buffer offset for the beginning of
		// the inserted range, as must be passed to OpenGL.
		GLintptr unmap() {
			assert(map_);
			buffer_->bind();
			glUnmapBuffer(buffer_->target());
			map_ = nullptr;
			return 0;
		}

		// Append an item to the buffer.
		template<typename... Args>
		void emplace_back(Args&&... args) {
			assert(map_);
			assert(count_ < max_);

			new(&map_[count_]) T(std::forward<Args>(args)...);
			count_++;
		}

		// Append space for count items, return a pointer to the first one.
		T* add(size_t count) {
			assert(count <= max_ && count_ <= max_ - count);
			T* ret = &map_[count_];
			count_ += count;
			return ret;
		}

		T& back() {
			assert(map_ && count_ >= 1);
			return map_[count_ - 1];
		}

	private:
		friend class StreamingBuffer;

		Inserter(StreamingBuffer& buffer, size_t max)
		  : buffer_(&buffer), map_(nullptr), count_(0), max_(max)
		{
			assert(!buffer_->inserting_);
			buffer_->inserting_ = true;

			buffer_->bind();
			map_ = reinterpret_cast<T*>(glMapBuffer(buffer_->target(), GL_WRITE_ONLY));
			if (!map_)
				throw wexception("Could not map GL buffer.");
		}

		StreamingBuffer* buffer_;
		T* map_;
		size_t count_;
		size_t max_;

		DISALLOW_COPY_AND_ASSIGN(Inserter);
	};

	StreamingBuffer(GLenum target) {
		target_ = target;
		glGenBuffers(1, &object_);
		if (!object_) {
			throw wexception("Could not create GL buffer.");
		}
	}

	~StreamingBuffer() {
		assert(!inserting_);

		if (object_) {
			glDeleteBuffers(1, &object_);
		}
	}

	// Returns the OpenGL object for direct use.
	GLuint object() const {
		return object_;
	}

	GLenum target() const {
		return target_;
	}

	void bind() const {
		glBindBuffer(target_, object_);
	}

	// Set the buffer up for streaming up to the given number of elements.
	//
	// Previous contents of the buffer are discarded (this does not affect
	// OpenGL functions that have been called previously).
	Inserter stream(size_t max) {
		// Always re-allocate the buffer. We rely on fast swap-out by the
		// driver. If backing store were to be shared globally, it might make
		// sense to consider an alternative scheme using unsynchronized maps
		// and explicit flushing.
		glBindBuffer(target_, object_);
		glBufferData(target_, sizeof(T) * max, NULL, GL_STREAM_DRAW);
		return Inserter(*this, max);
	}

private:
	GLenum target_;
	GLuint object_;
	bool inserting_ = false;

	DISALLOW_COPY_AND_ASSIGN(StreamingBuffer);
};

}  // namespace Gl

#endif  // end of include guard: WL_GRAPHIC_GL_STREAMING_BUFFER_H
