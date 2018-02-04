#DB_BUILD = deps/db-6.0.20/build_unix
DB_BUILD = deps/db-6.2.32/build_unix

all: build_db
	node-gyp build

clean:
	$(MAKE) -C $(DB_BUILD) clean
	node-gyp clean

config:
	mkdir -p $(DB_BUILD)
	node-gyp configure
	TOP=`pwd` && cd $(DB_BUILD) && export CFLAGS="-DSQLITE_ENABLE_FTS3=1 -DSQLITE_ENABLE_RTREE=1 -DSQLITE_ENABLE_FTS4=1 -DSQLITE_ENABLE_FTS3_PARENTHESIS=1 -DSQLITE_ENABLE_COLUMN_METADATA=1 -DSQLITE_ENABLE_UPDATE_DELETE_LIMIT=1 -DSQLITE_SOUNDEX=1 -DSQLITE_TEMP_STORE=1 -fPIC" && ../dist/configure --prefix=$$TOP --enable-debug  --enable-sql
	node-gyp configure -- -f xcode

build_db:
	$(MAKE) -C $(DB_BUILD) install


