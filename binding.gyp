{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "src/addon.cc",
        "src/db.cc",
        "src/dbenv.cc",
        "src/dbtxn.cc",
        "src/dbcursor.cc"
      ],
      "include_dirs": [
        "../include",
        "<!(node -e \"require('nan')\")",
        "./deps/db-6.2.32/build_unix"
      ],
      "link_settings": {
        "libraries": [
          "-L../lib",
          "-L../deps/db-6.2.32/build_unix",
          "-ldbsql-6.2"
        ]
      }
    }
  ]
}
