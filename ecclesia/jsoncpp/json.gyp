{
  'variables': {
    'src_root': '<(DEPTH)/third_party/jsoncpp',
  },
  'target_defaults': {
    'xcode_settings': {
      'GCC_PREFIX_HEADER': '',
    }
  },
  'targets': [
    {
      'target_name': 'json',
      'type': 'static_library',
      'include_dirs': [
        '<(DEPTH)',
      ],
      'sources': [
        '<(src_root)/json_reader.cc',
        '<(src_root)/json_value.cc',
        '<(src_root)/json_writer.cc',
      ],
    },
  ],
}
