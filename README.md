Chibitronics Love-to-Code Web Interface
=======================================

This is the source code to the Chibitronics Love-to-Code website.

It includes the modulator, as well as the user interface.  For modules, it uses the CodeMirror text editor, as well as a few other addons.

Audio is encoded into a Wav file, which is played back by the browser.

Building
--------

To build, install dependencies using npm:

    npm install

You may need to install the "gulp" program, in order to be able to build code:

    npm install -g gulp

Then build and serve using gulp:

    gulp watch