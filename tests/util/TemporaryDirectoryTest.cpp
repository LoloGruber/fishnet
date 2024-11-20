#include "Testutil.h"
#include <fishnet/TemporaryDirectiory.h>
#include <fstream>

using namespace testutil;
using namespace fishnet::util;

TEST(TemporaryDirectoryTest, init){
    size_t id = 0;
    EXPECT_NO_FATAL_FAILURE(id = TemporaryDirectory().id());
    TemporaryDirectory::load(id).transform([](auto&& value){value.clear();return value;}); // clean up if present
}

TEST(TemporaryDirectoryTest, getter){
    TemporaryDirectory tmp;
    EXPECT_NO_FATAL_FAILURE(tmp.id());
    EXPECT_EQ(tmp.get(),TemporaryDirectory::getTmpPrefix() / std::filesystem::path(std::to_string(tmp.id())+"/"));
    EXPECT_EQ(std::filesystem::path(tmp),tmp.get());
    tmp.clear();
}

TEST(TemporaryDirectoryTest, clear){
    TemporaryDirectory tmp;
    auto path = tmp / std::filesystem::path("Test.txt");
    std::ofstream ofs {path};
    ofs << "Hello World!";
    ofs.flush();  
    EXPECT_EXISTS(tmp.get());
    EXPECT_EXISTS(path);
    tmp.clear();
    EXPECT_NO_FATAL_FAILURE(tmp.clear()); //double clear should not cause problems
    EXPECT_NOT_EXISTS(tmp.get());
    EXPECT_NOT_EXISTS(path);
}

TEST(TemporaryDirectoryTest, clearAndInit){
    TemporaryDirectory tmp;
    EXPECT_EXISTS(tmp.get());
    tmp.clear();
    EXPECT_NOT_EXISTS(tmp.get());
    tmp.init();
    EXPECT_EXISTS(tmp.get());   
    tmp.clear();
}

TEST(TemporaryDirectoryTest, load){
    TemporaryDirectory tmp;
    auto copyOpt = TemporaryDirectory::load(tmp.id());
    EXPECT_VALUE(copyOpt);
    assert(copyOpt.has_value());
    auto copy = copyOpt.value();
    EXPECT_EQ(tmp.get(),copy.get());
    EXPECT_EQ(tmp.id(),copy.id());
    EXPECT_EXISTS(copy.get());
    copy.clear();
    EXPECT_NOT_EXISTS(copy.get());
    EXPECT_NOT_EXISTS(tmp.get());
}

TEST(TemporaryDirectoryTest, automaticClear){
    size_t id = 0;
    std::filesystem::path p;
    {
        AutomaticTemporaryDirectory tmp;
        EXPECT_EXISTS(tmp.get());
        id=tmp.id();
        p = tmp.get();
    }
    EXPECT_EMPTY(AutomaticTemporaryDirectory::load(id));
    EXPECT_NOT_EXISTS(p);
}

TEST(TemporaryDirectoryTest, loadAutomatic){
    TemporaryDirectory tmp;
    {
        auto copyOptAuto = AutomaticTemporaryDirectory::load(tmp.id());
        EXPECT_VALUE(copyOptAuto);
        assert(copyOptAuto.has_value());
        auto copyAuto = copyOptAuto.value();
        EXPECT_EXISTS(copyAuto.get());
    }
    EXPECT_NOT_EXISTS(tmp.get());
}

TEST(TemporaryDirectoryTest, implicitConversion){
    TemporaryDirectory tmp;
    EXPECT_EXISTS(tmp);
    tmp.clear();
    EXPECT_NOT_EXISTS(tmp);
}
