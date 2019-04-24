/*
 * Project: curve
 * Created Date: Thur Apr 16th 2019
 * Author: lixiaocui
 * Copyright (c) 2018 netease
 */

#include <gtest/gtest.h>
#include <glog/logging.h>
#include <memory>
#include <string>
#include "src/mds/nameserver2/namespace_storage_cache.h"
#include "src/mds/nameserver2/namespace_storage.h"
#include "src/common/timeutility.h"

namespace curve {
namespace mds {
TEST(CaCheTest, test_cache_with_capacity_limit) {
    int maxCount = 5;
    std::shared_ptr<LRUCache> cache = std::make_shared<LRUCache>(maxCount);

    // 1. 测试 put/get
    for (int i = 1; i <= maxCount + 1; i++) {
        cache->Put(std::to_string(i), std::to_string(i));
        std::string res;
        ASSERT_TRUE(cache->Get(std::to_string(i), &res));
        ASSERT_EQ(std::to_string(i), res);
    }

    // 2. 第一个元素被剔出
    std::string res;
    ASSERT_FALSE(cache->Get(std::to_string(1), &res));
    for (int i = 2; i <= maxCount + 1; i++) {
        ASSERT_TRUE(cache->Get(std::to_string(i), &res));
        ASSERT_EQ(std::to_string(i), res);
    }

    // 3. 测试删除元素
    cache->Remove("1");
    cache->Remove("2");
    ASSERT_FALSE(cache->Get("2", &res));

    // 4. 重复put
    cache->Put("4", "hello");
    ASSERT_TRUE(cache->Get("4", &res));
    ASSERT_EQ("hello", res);
}

TEST(CaCheTest, test_cache_with_capacity_no_limit) {
    std::shared_ptr<LRUCache> cache = std::make_shared<LRUCache>();

    // 1. 测试 put/get
    std::string res;
    for (int i = 1; i <= 10; i++) {
        cache->Put(std::to_string(i), std::to_string(i));
        ASSERT_TRUE(cache->Get(std::to_string(i), &res));
        ASSERT_EQ(std::to_string(i), res);
    }

    // 2. 测试元素删除
    cache->Remove("1");
    ASSERT_FALSE(cache->Get("1", &res));
}
TEST(CaCheTest, test_cache_with_large_data_capacity_no_limit) {
    std::shared_ptr<LRUCache> cache = std::make_shared<LRUCache>();

    int i = 1;
    FileInfo fileinfo;
    std::string filename = "helloword-" + std::to_string(i) + ".log";
    fileinfo.set_id(i);
    fileinfo.set_filename(filename);
    fileinfo.set_parentid(i << 8);
    fileinfo.set_filetype(FileType::INODE_PAGEFILE);
    fileinfo.set_chunksize(DefaultChunkSize);
    fileinfo.set_length(10 << 20);
    fileinfo.set_ctime(::curve::common::TimeUtility::GetTimeofDayUs());
    std::string fullpathname = "/A/B/" + std::to_string(i) + "/" + filename;
    fileinfo.set_fullpathname(fullpathname);
    fileinfo.set_seqnum(1);
    std::string encodeFileInfo;
    ASSERT_TRUE(fileinfo.SerializeToString(&encodeFileInfo));
    std::string encodeKey =
            NameSpaceStorageCodec::EncodeFileStoreKey(i << 8, filename);

    // 1. put/get
    cache->Put(encodeKey, encodeFileInfo);
    std::string out;
    ASSERT_TRUE(cache->Get(encodeKey, &out));
    FileInfo fileinfoout;
    ASSERT_TRUE(NameSpaceStorageCodec::DecodeFileInfo(out, &fileinfoout));
    NameSpaceStorageCodec::DecodeFileInfo(out, &fileinfoout);
    ASSERT_EQ(filename, fileinfoout.filename());
    ASSERT_EQ(fullpathname, fileinfoout.fullpathname());

    // 2. remove
    cache->Remove(encodeKey);
    ASSERT_FALSE(cache->Get(encodeKey, &out));
}

}  // namespace mds
}  // namespace curve


