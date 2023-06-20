# Cuckoo Counter's Redis Integration

## How to run

Suppose you've already cloned the repository.

First, you need to install Redis first. You can use the "redis-6.2.6.tar.gz" file in this folder, or you can go to the redis official website to download the latest version.

Unzip and install:

$ tar xzf redis-6.2.6.tar.gz
$ cd redis-6.2.6
$ make

After executing the $make command, the compiled "redis-server" and "redis-cli" will appear in "./src" folder.

Next, you need to add a line to "redis.conf" file:

-->	loadmodule ../RedisBloom/redisbloom.so


Then start the Redis server with the configuration file:

$ cd ./src
$ ./redis-server ../redis.conf

After starting the Redis server process, you can use the test client program "redis-cli" to interact with Redis server. Open another terminal:

$ cd src
$ ./redis-cli
redis> cc.create CC_1 4000
OK
redis> cc.insert CC_1 flow1 flow1
1) (nil)
2) (nil)
redis> cc.query flow1
1) (integer) 2

Currently supported commands are:

1. cc.create KEY WIDTH
  KEY: The name of the cuckoo counter.
  WIDTH: Number of buckets in each array.

2. cc.insert KEY ITEM [ITEM...]
  KEY: The name of the cuckoo counter.
  ITEM: The item which is inserted into cuckoo counter.

3. cc.query KEY ITEM [ITEM...]
  KEY: The name of the cuckoo counter.
  ITEM: One or more items for which to return the count.
