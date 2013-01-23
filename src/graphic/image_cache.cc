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

#include "log.h"
#include "picture.h"
#include "picture_impl.h"
#include "image_loader.h"

#include "image_cache.h"

using namespace std;

namespace  {

// Implementation stuff {{{

// End: Implementation stuff }}}
class ImageImplFromDisk;
class ImageCacheImpl : public ImageCache {
public:
	// Ownership of loader is not taken.
	ImageCacheImpl(IImageLoader* loader) : img_loader_(loader) {}
	~ImageCacheImpl();

	// Implements ImageCache
	virtual const IPicture* get(PicMod, const string& hash) const;
	virtual const IPicture* insert(PicMod, const string& hash, const IPicture*);

	// NOCOM(#sirver): load should not be here. flush
	virtual const IPicture* load(PicMod, const string& fn, bool alpha);
	virtual void flush(PicMod);

private:
	struct PictureRec {
		const IPicture* picture;
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
	return it->second.picture;
}

const IPicture* ImageCacheImpl::insert(PicMod module, const string& name, const IPicture* pic) {
	PictureRec rec;
	rec.picture = new IPicture(pic;
	m_picturemap.insert(make_pair(name, rec));
	return pic;
}

const IPicture* ImageCacheImpl::load
		(PicMod module, const string& fname, bool alpha)
{
	//  Check if the picture is already loaded.
	PictureMap::iterator it = m_picturemap.find(fname);
	if (it == m_picturemap.end()) {
		// NOCOM(#sirver): PictureRec is not really needed anymore.
		PictureRec rec;
		rec.image = new ImageImplFromDisk(this, picmod, fname, alpha);
		it = m_picturemap.insert(make_pair(fname, rec)).first;
	}

	if (!img->has_surface()) {
		log("ImgCache: Loading %s from disk.\n", fname.c_str());
		ImageImpl* img = img_loader_->load(fname, alpha);
		img->set_surface(img);
	}

	return it->second.picture;
}

void ImageCacheImpl::flush(PicMod module) {
	return; // NOCOM(#sirver): this does nothing currently
	vector<string> eraselist;

	BOOST_FOREACH(PictureMap::value_type& entry, m_picturemap) {
		eraselist.push_back(entry.first);
	}

	while (!eraselist.empty()) {
		m_picturemap.erase(eraselist.back());
		eraselist.pop_back();
	}
}

}  // namespace

ImageCache* create_image_cache(IImageLoader* loader) {
	return new ImageCacheImpl(loader);
}

