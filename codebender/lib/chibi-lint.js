// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: http://codemirror.net/LICENSE

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
"use strict";

CodeMirror.registerHelper("lint", "text/x-c++src", function(text, passOptions, cm) {
  var found = [];
  var internal_error_str = "";

  var error_message = cm.chibi_error_string;
  if (!error_message) {
    cm.last_internal_error_str = internal_error_str;
    return found;
  }

  var lines = error_message.split("\n");
  var i;

  console.log("Error message: " + error_message);
  // The number of "unknown error" lines we can skip.
  // We set this because error messages tend to give two lines of context,
  // which we can ignore.
  var skip_lines = 0;
  for (i = 0; i < lines.length; i++) {
    var line = lines[i];
    var inofilestr = "(sketch file) ";
    var exitstatusstr = "exit status ";
    var compileterm = "compilation terminated";
    if (line.substr(0, inofilestr.length) === inofilestr) {
      var parts = line.split(":");
      var filename = parts.shift();
      var lineno = parts.shift() - 1;
      var error = parts.join(":");

      found.push({ from: CodeMirror.Pos(lineno, 0), to: CodeMirror.Pos(lineno, 0), message: error });
      skip_lines = 2;
    }

    // Just ignore the "exit status 1" line
    else if (line.substr(0, exitstatusstr.length) === exitstatusstr) {
      ;
    }

    // Also ignore the "compilation terminated" line
    else if (line.substr(0, compileterm.length) === compileterm) {
      ;
    }

    // Ignore "changing start of text section" messages.
    else if (line.indexOf("warning: changing start of section .text by 4 bytes") > -1) {
      ;
    }

    // Ignore "[filename]: In function [function]" messages, since they're followed by the error
    else if (line.indexOf(": In function ") > -1) {
      ;
    }

    // Ignore blank lines.
    else if (!line) {
      ;
    }

    // If we have lines we can skip, ignore those too.
    else if (skip_lines > 0) {
      skip_lines--;
    }

    else {
      internal_error_str += "\n" + line;
    }
  }

  if ((internal_error_str !== "") && (internal_error_str !== cm.last_internal_error_str)) {
    $("#internal_error_text").text(internal_error_str);
    $("#internal_error_dialog").dialog("open");
    cm.last_internal_error_str = internal_error_str;
  }
  return found;
});

});
