/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <string>
#include <map>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

#include "picture.h"

#include "image_loader.h"
#include "image_cache.h"

using namespace std;

class ImageCacheImpl : public ImageCache {
public:
	// Ownership of loader is not taken.
	ImageCacheImpl(IImageLoader* loader) : img_loader_(loader) {}
	~ImageCacheImpl();

	// Implements ImageCache
	virtual const IPicture* get(PicMod, const string& hash) const;
	virtual const IPicture* insert(PicMod, const string& hash, const IPicture*);
	virtual const IPicture* load(PicMod, const string& fn, bool alpha);
	virtual void flush(PicMod);

private:
	struct PictureRec {
		const IPicture* picture;

		/// bit-mask of modules that this picture exists in
		uint32_t modules;
	};

	typedef map<string, PictureRec> PictureMap;

	/// hash of cached filename/picture pairs
	PictureMap m_picturemap;

	IImageLoader* img_loader_;
};

ImageCacheImpl::~ImageCacheImpl() {
	BOOST_FOREACH(PictureMap::value_type& p, m_picturemap)
		delete p.second.picture;
	m_picturemap.clear();
}

const IPicture* ImageCacheImpl::get(PicMod module, const string& hash) const {
	PictureMap::const_iterator it = m_picturemap.find(hash);
	if (it == m_picturemap.end())
		return NULL;
	return (it->second.modules & (1 << module)) ? it->second.picture : NULL;
}

const IPicture* ImageCacheImpl::insert(PicMod module, const string& name, const IPicture* pic) {
	PictureRec rec;
	rec.picture = pic;
	rec.modules = 1 << module;
	m_picturemap.insert(make_pair(name, rec));
	return pic;
}

void ImageCacheImpl::flush(PicMod module) {
	vector<string> eraselist;

	BOOST_FOREACH(PictureMap::value_type& entry, m_picturemap) {
		entry.second.modules &= ~(1 << module);
		if (!entry.second.modules)
			eraselist.push_back(entry.first);
	}

	while (!eraselist.empty()) {
		m_picturemap.erase(eraselist.back());
		eraselist.pop_back();
	}
}

const IPicture* ImageCacheImpl::load
		(PicMod module, const string& fname, bool alpha)
{
	//  Check if the picture is already loaded.
	PictureMap::iterator it = m_picturemap.find(fname);

	if (it == m_picturemap.end()) {
		PictureRec rec;

		rec.picture = img_loader_->load(fname, alpha);
		rec.modules = 0;

		it = m_picturemap.insert(make_pair(fname, rec)).first;
	}

	it->second.modules |= 1 << module;
	return it->second.picture;
}

ImageCache* create_image_cache(IImageLoader* loader) {
	return new ImageCacheImpl(loader);
}
