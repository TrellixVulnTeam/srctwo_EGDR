# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'shortcut_util',
      'dependencies': [
        '<(DEPTH)/ui/webui/resources/js/compiled_resources2.gyp:assert',
        '<(DEPTH)/ui/webui/resources/js/compiled_resources2.gyp:cr',
      ],
      'includes': ['../../../../third_party/closure_compiler/compile_js2.gypi'],
    },
    {
      'target_name': 'drag_and_drop_handler',
      'dependencies': [
        '<(DEPTH)/ui/webui/resources/js/compiled_resources2.gyp:assert',
        '<(DEPTH)/ui/webui/resources/js/compiled_resources2.gyp:cr',
        '<(DEPTH)/ui/webui/resources/js/cr/ui/compiled_resources2.gyp:drag_wrapper',
      ],
      'includes': ['../../../../third_party/closure_compiler/compile_js2.gypi'],
    },
    {
      'target_name': 'extensions',
      'variables': {
        'script_args': ['--custom_sources'],
        'source_files': [
          '<(DEPTH)/ui/webui/resources/js/promise_resolver.js',
          '<(DEPTH)/ui/webui/resources/js/load_time_data.js',
          '<(DEPTH)/ui/webui/resources/js/cr.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/array_data_model.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/list.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/focus_outline_manager.js',
          'extension_loader.js',
          '<(DEPTH)/ui/webui/resources/js/util.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/focus_manager.js',
          'extension_focus_manager.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/list_item.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/focus_row.js',
          'extension_options_overlay.js',
          '<(DEPTH)/third_party/closure_compiler/externs/management.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/list_selection_controller.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/alert_overlay.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/bubble.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/focus_grid.js',
          'chromeos/kiosk_apps.js',
          '<(DEPTH)/ui/webui/resources/js/assert.js',
          'extension_commands_overlay.js',
          'extensions.js',
          'extension_list.js',
          'chromeos/kiosk_app_disable_bailout_confirm.js',
          'focus_row.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/bubble_button.js',
          '<(DEPTH)/third_party/closure_compiler/externs/chrome_send.js',
          '<(DEPTH)/third_party/jstemplate/util.js',
          'extension_error.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui.js',
          '<(DEPTH)/ui/webui/resources/js/event_tracker.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/overlay.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/drag_wrapper.js',
          'extension_code.js',
          '<(DEPTH)/ui/webui/resources/js/cr/event_target.js',
          '<(DEPTH)/third_party/jstemplate/jsevalcontext.js',
          'extension_command_list.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/controlled_indicator.js',
          'extension_error_overlay.js',
          'drag_and_drop_handler.js',
          '<(DEPTH)/ui/webui/resources/js/cr/ui/list_selection_model.js',
          '<(DEPTH)/third_party/jstemplate/jstemplate.js',
          'chromeos/kiosk_app_list.js',
          '<(DEPTH)/third_party/closure_compiler/externs/developer_private.js',
          'shortcut_util.js',
          'pack_extension_overlay.js',
        ],
      },
      'includes': ['../../../../third_party/closure_compiler/compile_js2.gypi'],
    }
  ]
}
