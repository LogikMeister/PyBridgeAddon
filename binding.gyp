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
          }
        }]
      ],
			"include_dirs": [
				"<(module_root_dir)/include",
				"<(module_root_dir)/include/python38"
			],
			"libraries": [
				"<(module_root_dir)/lib/python3.lib",
				"<(module_root_dir)/lib/python38.lib"
			]
    }
  ]
}