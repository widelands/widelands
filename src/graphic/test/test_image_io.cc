#include <memory>
#include <set>
#include <string>

#define BOOST_TEST_MODULE ImageIo

#include <boost/test/unit_test.hpp>

#include "graphic/pam_io.h"
#include "graphic/test/paths.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"

struct ImageIoTestFixture {
	ImageIoTestFixture() {
		g_fs = new LayeredFileSystem();
		g_fs->AddFileSystem(&FileSystem::Create(GRAPHIC_TEST_DATADIR));
	}

	~ImageIoTestFixture() {
		delete g_fs;
	}

};

BOOST_FIXTURE_TEST_SUITE(image_io_test_suite, ImageIoTestFixture)

BOOST_AUTO_TEST_CASE(TestLoadAndSave) {
	std::unique_ptr<Surface> surface;

	{
		FileRead fr;
		fr.Open(*g_fs, "data/wl-ico-32.pam");
		surface = load_pam(&fr);
	}
	BOOST_CHECK(surface != nullptr);

	FileWrite fw;
	save_pam(surface.get(), &fw);

	{
		FileRead fr;
		fr.Open(*g_fs, "data/wl-ico-32.pam");
		const std::string from_disk(fr.Data(fr.GetSize()), fr.GetSize());
		BOOST_CHECK_EQUAL(from_disk, fw.GetData());
	}
}

BOOST_AUTO_TEST_SUITE_END();
