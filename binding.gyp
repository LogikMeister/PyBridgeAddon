{
  "targets": [
    {
      "target_name": "pybridge",
      "sources": ["src/cpp/binding.cpp"],
      "cflags!": [ "-fno-exceptions, -std=c++17" ],
      "cflags_cc!": [ "-fno-exceptions, -std=c++17" ],
      "conditions": [
        ["OS=='win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalOptions": ["/std:c++17"]
            }
          },
          "include_dirs": [
            "<(module_root_dir)/include",
            "<!(node ./src/script/win.js include)"
          ],
          "libraries": [
            "<!(node ./src/script/win.js libs)"
          ]
        }]
      ]
    }
  ]
}