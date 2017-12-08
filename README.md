Chibitronics Love-to-Code Web Interface
=======================================

This is the source code to the Chibitronics Love-to-Code website.

It includes the modulator, as well as the user interface.  For modules, it uses the CodeMirror text editor, as well as a few other addons.

Audio is encoded into a Wav file, which is played back by the browser.

Building
--------

[![Build Status](https://travis-ci.org/chibitronics/ltc-webview-layer.svg?branch=master)](https://travis-ci.org/chibitronics/ltc-webview-layer)

To build, install dependencies using npm:

    npm install

You may need to install the "gulp" program, in order to be able to build code:

    npm install -g gulp

Then build and serve using gulp:

    gulp build
    gulp watch

Note that the example menu needs to be hand-edited in src/index.html

Deploying
---------

To deploy, rebuild the project using the "default" target:

    gulp

You can then copy the contents of "build/", or serve it using "gulp serve-prod":

    gulp serve-prod


Docker Support
==============

This package has full Docker support, and will rebuild itself and serve files on port 80.  To build a Docker image, simply run "docker build .".  You may want to specify a tag:

    docker build -t ltc-web-ui .

You can then run it locally, exposing it on port 8080:

    docker run --rm -it -p 8080:80 ltc-web-ui

By default, it uses the build server at //compile.ltc.chibitronics.com and you can change this by setting COMPILE_URL:

    docker run --rm -it -p 8080:80 -e COMPILE_URL=//compile.example.com/ ltc-web-ui