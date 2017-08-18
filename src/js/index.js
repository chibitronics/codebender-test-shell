var ModulationController = require('chibitronics-ltc-modulate');
var CodeMirror = require('codemirror');
require('codemirror/mode/clike/clike');
require('codemirror/addon/lint/lint');
require('./chibi-lint.js');

var editor;
var codeobj = {};
var modController;
var autosaveGeneration = null;
var embedMode = false;

var lastText = null;

// Piwik tracking interface
window._paq = window._paq || [];

// Default 'save' filename
var fileName = 'LTC-program.ino';

var isIE11 = /Trident.*rv[ :]*11\./.test(navigator.userAgent);

// This gets replaced by reading /config.json at runtime.
var config = {
    compileUrl: '//ltc.xobs.io/compile'
};

/* Internet Explorer doesn't support WAV, so render an MP3 */
var audioFormat = 'wav';
if (isIE11) {
    /* LameJS is LGPL, so link it separately, and only if necessary */
    /*jslint evil: true */
    document.write('<script language="javascript" type="text/javascript" src="js/lame.min.js"></script>');

    /* IE doesn't support log10, so polyfill it */
    Math.log10 = function(x) {
        return Math.log(x) / Math.LN10;
    };

    audioFormat = 'mp3';
}

function getAudioElement() {
    return document.getElementById('audio_output');
}

function getCanvas() {
    return document.getElementById('wavStrip');
}

function getWaveFooter() {
    return document.getElementById('Site-footer');
}

function buildResult(results, textStatus, status, jqXHR) {
    if (status !== 200) {
        console.log('Don\'t know what to do.  Backend failure?');
        window._paq.push(['trackEvent', 'Compile', 'internal-error', textStatus, status]);
        return;
    }

    results = JSON.parse(results);

    if (results.success) {
        document.getElementById('buildoutput').innerHTML = 'Build complete';

        // We do enjoy it when code compiles successfully.
        window._paq.push(['trackEvent', 'Compile', 'success']);

        var data = atob(results.output);
        var dataU8 = new Uint8Array(data.length);
        for (var i = 0; i < data.length; i++) {
            dataU8[i] = data.charCodeAt(i);
        }

        if (modController) {
            modController.stop();
        }

        var lbrEnable = document.getElementById('lbr_button').checked;
        var modulationVersion = 2; // force to v2 modulation all the time
        //if (document.getElementById('mod_version').checked) {
        //    modulationVersion = 2;
        //}

        modController = new ModulationController({
            canvas: getCanvas(),
            lbr: lbrEnable,
            uriType: 'data',
            format: audioFormat,
            endCallback: function() {
                getWaveFooter().style.display = 'none';
            }
        });

        modController.transcodeToAudioTag(dataU8, getAudioElement());

        getWaveFooter().style.display = 'block';
    } else {
        window._paq.push(['trackEvent', 'Compile', 'fail', 'errors', results.log.length]);
        editor.chibiErrorString = results.message;
        editor.performLint();
    }
}

function clickUpload(e) {
    selectTab(e);
    document.getElementById('buildoutput').innerHTML = 'Building code...';

    // Disable the "Upload" button to prevent spamming the server.
    var uploadButton = document.getElementById('upload_button');
    uploadButton.classList.add('uploading');
    uploadButton.onclick = undefined;

    // Play empty data onclick to enable audio playback.
    var audioTag = getAudioElement();
    if (isIE11) {
        audioTag.src = 'data:audio/mpeg;base64,' +
            'SUQzBAAAAAAAI1RTU0UAAAAPAAADTGF2Z' +
            'jU2LjM2LjEwMAAAAAAAAAAAAAAA//OEAA' +
            'AAAAAAAAAAAAAAAAAAAAAASW5mbwAAAA8' +
            'AAAAEAAABIADAwMDAwMDAwMDAwMDAwMDA' +
            'wMDAwMDAwMDV1dXV1dXV1dXV1dXV1dXV1' +
            'dXV1dXV1dXV6urq6urq6urq6urq6urq6u' +
            'rq6urq6urq6v/////////////////////' +
            '///////////8AAAAATGF2YzU2LjQxAAAA' +
            'AAAAAAAAAAAAJAAAAAAAAAAAASDs90hvA' +
            'AAAAAAAAAAAAAAAAAAA//MUZAAAAAGkAA' +
            'AAAAAAA0gAAAAATEFN//MUZAMAAAGkAAA' +
            'AAAAAA0gAAAAARTMu//MUZAYAAAGkAAAA' +
            'AAAAA0gAAAAAOTku//MUZAkAAAGkAAAAA' +
            'AAAA0gAAAAANVVV';
    } else {
        audioTag.src = 'data:audio/wav;base64,' +
            'UklGRigAAABXQVZFZm10IBIAAAABAAEAR' +
            'KwAAIhYAQACABAAAABkYXRhAgAAAAEA';
    }
    audioTag.play();

    if (modController) {
        modController.stop();
    }

    codeobj = {
        'files': [{
            'filename': 'project.ino',
            'content': editor.getValue()
        }],
        'format': 'binary',
        'version': '167',
        'libraries': [],
        'fqbn': 'chibitronics:esplanade:code'
    };

    // Clear out the old error message.
    editor.chibiErrorString = '';
    editor.performLint();

    var request = new window.XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState === 4) {
            // Re-enable the "Upload" button.
            var uploadButton = document.getElementById('upload_button');
            uploadButton.classList.remove('uploading');
            uploadButton.onclick = clickUpload;

            // Dispatch the "build complete" message.
            buildResult(request.responseText, request.statusText, request.status, request);
        }
    };
    request.open('POST', config.compileUrl, true);
    window._paq.push(['trackEvent', 'Compile', 'start']);
    request.send(JSON.stringify(codeobj));

    return false;
}

function getLocalSketches() {
    // Get the previous array of sketches
    var localSketches = localStorage.getItem('sketches');
    if (localSketches) {
        return JSON.parse(localSketches);
    }

    return {};
}

function saveLocalSketches(localSketches) {
    localStorage.setItem('sketches', JSON.stringify(localSketches));
}

//function getGithubToken() {
//    return localStorage.getItem('gitHubToken');
//}

function saveCurrentEditor() {
    // If this is the first run, autosaveGeneration will be null.
    // If it's not the first run, then we can see whether the
    // document has changed since the last generation.'
    if (autosaveGeneration === null) {
        autosaveGeneration = editor.changeGeneration();
    } else if (!editor.isClean(autosaveGeneration)) {
        if (embedMode) {
            return;
        }
        localStorage.setItem('currentSketch', editor.getValue());
        autosaveGeneration = editor.changeGeneration();
    }

    // Run autosave again in five seconds.
    window.setTimeout(saveCurrentEditor, 5000);
}

/*
function loadSavedEditor() {
    editor.setValue(localStorage.getItem('currentSketch'));
}
*/

function downloadSketch(contents, fileName) {
    // Automatically download the sketch.
    // Note: this is a bit annoying, so we may disable it.
    var blob = new Blob([contents], { type: 'application/octet-binary' });
    var a = document.createElement('a');
    document.body.appendChild(a);
    a.style = 'display: none';
    var url = (window.URL || window.webkitURL).createObjectURL(blob);
    a.href = url;

    if (fileName === '') {
        fileName = 'LTC-program.ino';
    }
    if (!fileName.endsWith('.ino')) {
        fileName = fileName + '.ino';
    }
    a.download = fileName;
    a.click();

    // Remove the URL in 100 ms, enough time for the downloader to run.
    setTimeout(function() {
        document.body.removeChild(a);
        (window.URL || window.webkitURL).revokeObjectURL(url);
    }, 100);
}

function storeSketchLocally(contents, sketchName) {
    var localSketches = getLocalSketches();

    // Store the document in the document list
    localSketches[sketchName] = {
        'name': sketchName,
        'document': contents
    };

    // Stash the document list back in local storage
    saveLocalSketches(localSketches);

    // Re-populate the sketch list
    // TODO: Simply add the new sketch instead of redoing everything
    populateSketchList();
    populateSaveMenu();

    selectTab('code_editor');
    editor.refresh();

    downloadSketch(editor.getValue(), sketchName);

    updateTextChangeBuffer(); // note the save
}

function saveLocalSketchAs(e) {
    var sketchName = document.getElementById('saveNewSketchName').value;
    storeSketchLocally(editor.getValue(), sketchName);

    // Don't let the form submit.
    return false;
}

function overwriteSketch(e) {
    var sketchName = e.target.sketchName;
    storeSketchLocally(editor.getValue(), sketchName);

    // Don't let the form submit.
    return false;
}

function initializeEditor() {
    var editorNode = document.getElementById('code_editor_textarea');

    var savedSketch = localStorage.getItem('currentSketch');
    if (savedSketch) {
        editorNode.innerHTML = savedSketch;
    }

    editor = CodeMirror.fromTextArea(editorNode, {
        value: editorNode.value,
        lineNumbers: true,
        matchBrackets: true,
        mode: 'text/x-c++src',
        scrollbarStyle: 'null',
        lint: true,
        gutters: ['CodeMirror-lint-markers'],
        useCPP: true
    });
    var mac = (CodeMirror.keyMap['default'] === CodeMirror.keyMap.macDefault);
    CodeMirror.keyMap['default'][(mac ? 'Cmd' : 'Ctrl') + '-Space'] = 'autocomplete';

    saveCurrentEditor();
}

function resizeHeader() {
    checkRainbow(editor.getValue());
    var header = document.getElementById('Site-header');
    var height = header.offsetHeight;
    document.getElementById('main').style.marginTop = height + 'px';
    header.style.top = 0;
}

function installHooks() {
    window.onresize = resizeHeader;
    document.getElementById('upload_button').onclick = clickUpload;
    document.getElementById('examples_button').onclick = selectTab;
    document.getElementById('saveas_button').onclick = selectTab;
    document.getElementById('load_button').onclick = selectTab;
}

function hideShowExampleCategory(e) {
    var elem = e.target;
    if (elem.nextElementSibling.style.display === '') {
        elem.nextElementSibling.style.display = 'none';
    } else {
        elem.nextElementSibling.style.display = '';
    }
    return false;
}

function selectTab(e) {

    var target;
    var elements = document.getElementsByClassName('maintab');

    // If we're called with a string as a parameter, just set that tab
    if ((typeof e) === 'string') {
        target = e;
    }
    // Otherwise, treat it as an element, and select that tab.
    else {
        target = e.target.attributes['data-target'].value;
    }

    var found = false;
    var showWarning = true;
    for (var i = 0; i < elements.length; i++) {
        var element = elements[i];

        if (element.id === target) {
            // If the item is already highlighted, we want to switch
            // back to the default view.
            // So as a hack, make this item invisible, and don't mark
            // the 'true' flag.  Instead, fall through below, and allow
            // the default view to be shown.
            if (element.style.display === 'block') {
                element.style.display = 'none';
                showWarning = false;
            } else {
                element.style.display = 'block';
                found = true;
            }
        } else {
            element.style.display = 'none';
        }
    }

    // If no item was found, make the code editor visible by default
    if (!found) {
        if (showWarning) {
            console.log('Warning: Unrecognized element ' + target +
                ', selecting code_editor by default');
        }
        document.getElementById('code_editor').style.display = 'block';
    }
}

function checkRainbow(s) {
    var hasrainbow = s.search(/rainbow/i);

    var gadget = document.getElementById('color_gadget');
    if (hasrainbow !== -1) {
        gadget.style.display = 'block';
    } else {
        gadget.style.display = 'none';
    }
}

function loadExampleFromLink(e) {

    var request = new window.XMLHttpRequest();
    var target = e.target;

    if (hasTextChanged()) {
        var retVal = confirm('Unsaved changes! Proceed and lose changes?');
        if (retVal === false) {
            selectTab('code_editor');
            return false;
        }
    }

    // If we clicked on the <LI> outside of the link, manually select the <A> tag inside.
    if (e.target.tagName === 'LI') {
        target = target.firstChild;
    }

    request.onreadystatechange = function() {
        if (request.readyState === 4) {

            var editorBox = document.getElementById('code_editor');
            var examplesBox = document.getElementById('examples_list');

            examplesBox.style.display = 'none';
            editorBox.style.display = 'block';

            if (request.status === 200) {
                editor.setValue(request.responseText);
                resizeHeader();
                updateTextChangeBuffer(); // note the new file is loaded from stock image
            }
        }
    };
    try {
        // Log the selected example
        var parentCategoryName = target.parentElement.parentElement.parentElement.previousElementSibling.textContent;
        var exampleName = target.textContent;
        window._paq.push(['trackEvent', 'Example', 'Load', parentCategoryName + ' / ' + exampleName]);
    } catch (e) {}
    request.open('GET', target.href, true);
    request.send();

    return false;
}

function fixupExamples() {
    if (embedMode) {
        return;
    }
    var i;
    var e;
    var exampleCategories = document.getElementsByClassName('ExampleCategory');
    for (i = 0; i < exampleCategories.length; i++) {
        e = exampleCategories[i];

        // Mark all child categories as hidden
        e.nextElementSibling.style.display = 'none';

        // Set it up so we can click to expand
        e.onclick = hideShowExampleCategory;
    }

    var exampleItems = document.getElementsByClassName('ExampleItem');
    for (i = 0; i < exampleItems.length; i++) {
        e = exampleItems[i];

        e.firstChild.onclick = function() { return false; };
        e.onclick = loadExampleFromLink;
    }
}

function undoDeleteLocalSketch(a) {
    var localSketches = getLocalSketches();
    var deletedSketch = localStorage.getItem('deletedSketch');

    // In theory, we can only get here if deletedSketch exists.  If it doesn't,
    // then that's really weird.
    if (deletedSketch === undefined) {
        console.log('No deleted sketch found');
        return false;
    }
    deletedSketch = JSON.parse(deletedSketch);

    // Load the sketch back into the local sketch list, and remove it from 'undo'
    localSketches[deletedSketch.name] = deletedSketch;
    localStorage.removeItem('deletedSketch');

    saveLocalSketches(localSketches);

    populateSketchList();
    populateSaveMenu();

    return false;
}

function deleteLocalSketch(a) {
    var localSketches = getLocalSketches();
    var sketchName = a.target.sketchName;

    // Store the deleted sketch in a temporary location, for Undo
    var deletedSketch = localSketches[sketchName];
    localStorage.setItem('deletedSketch', JSON.stringify(deletedSketch));

    // Delete the sketch from the database, and commit the database
    delete localSketches[sketchName];
    localStorage.setItem('sketches', JSON.stringify(localSketches));

    // Redraw the list of sketches, now that one is deleted
    populateSketchList();
    populateSaveMenu();

    // Add in an 'Undo Delete' link
    var li = document.createElement('li');
    var undo = document.createElement('span');
    var hr = document.createElement('hr');
    li.appendChild(hr);
    undo.className = 'SketchItem';
    undo.innerHTML = 'Undo Delete';
    undo.onclick = undoDeleteLocalSketch;
    undo.setAttribute('class', 'teal_button');
    li.appendChild(undo);
    document.getElementById('sketch_list').firstChild.appendChild(li);

    return false;
}

function loadLocalSketch(e) {

    var localSketches = getLocalSketches();
    var sketchName = e.target.sketchName;

    if (hasTextChanged()) {
        var retVal = confirm('Unsaved changes! Proceed and lose changes?');
        if (retVal === false) {
            selectTab('code_editor');
            return false;
        }
    }

    var sketch = localSketches[sketchName];
    if (sketch === undefined) {
        console.log('Couldn\'t find local sketch ' + sketchName);
        return false;
    }

    editor.setValue(sketch.document);
    resizeHeader();

    selectTab('code_editor');
    editor.refresh();

    return false;
}

function startDownloadSketch(e) {
    downloadSketch(editor.getValue(), document.getElementById('download_name').value);
    selectTab('code_editor');
    return false;
}

function uploadSketch(e) {
    var elt = e.target;
    var files = elt.files;

    if (hasTextChanged()) {
        var retVal = confirm('Unsaved changes! Proceed and lose changes?');
        if (retVal === false) {
            selectTab('code_editor');
            return false;
        }
    }

    if (files.length < 1) {
        console.log('No files were selected');
        return false;
    }

    var file = files[0];

    // Make sure the file is a sane size.
    if (file.size > 1024 * 1024) {
        console.log('File is way too big (1MB limit)');
        return false;
    }

    var reader = new FileReader();
    reader.onload = (function(contents) {
        return function(e) {
            editor.setValue(e.target.result);
            resizeHeader();

            selectTab('code_editor');
            editor.refresh();
            elt.value = null;
        };
    })(file);
    reader.readAsText(file);

    // Eat the "click" event.
    return false;
}

function populateSketchList() {
    if (embedMode) {
        return;
    }
    var localSketches = getLocalSketches();
    var li;
    var a;
    var t;
    var note;

    var sketchList = document.getElementById('sketch_list');

    // Remove all previous child nodes, in case we're re-populating the list.
    var childNodes = sketchList.childNodes;
    for (var i = 0; i < childNodes.length; i++) {
        sketchList.removeChild(childNodes[i]);
    }

    // Create an unordered list to store the sketch list
    var ul = document.createElement('ul');
    ul.className = 'SketchList';

    // Add sketches stored in localStorage
    for (var name in localSketches) {
        if (!localSketches.hasOwnProperty(name)) {
            continue;
        }
        li = document.createElement('li');
        li.className = 'SketchItem';

        var s = document.createElement('span');
        s.sketchName = name;
        s.innerHTML = name;

        var b = document.createElement('a');
        b.setAttribute('class', 'teal_button');
        b.innerHTML = 'Load';
        b.sketchName = name;
        b.onclick = loadLocalSketch;

        a = document.createElement('a');
        a.setAttribute('class', 'red_button');
        a.sketchName = name;
        a.onclick = deleteLocalSketch;
        a.innerHTML = 'Delete';

        var s2 = document.createElement('span');
        s2.innerHTML = ' ';
        var s3 = document.createElement('span');
        s3.innerHTML = ' ';

        li.appendChild(s);
        li.appendChild(s2);
        li.appendChild(b);
        li.appendChild(s3);
        li.appendChild(a);
        ul.appendChild(li);
    }

    {
        li = document.createElement('li');
        var hr = document.createElement('hr');
        li.appendChild(hr);
        ul.appendChild(li);
    }

    // Add an entry for GitHub, or add a signup link if one doesn't exists
    /*
    if (getGithubToken() === null) {
        li = document.createElement('li');

        a = document.createElement('a');
        a.innerHTML = 'Connect to GitHub';
        a.setAttribute('href', '/connectToGitHub');
        a.setAttribute('target', '__new');
        a.setAttribute('class', 'teal_button');

        li.appendChild(a);
        ul.appendChild(li);
    }
    */

    {
        li = document.createElement('li');

        note = document.createElement('p');
        t = document.createTextNode('Click upload to select files stored on your device and load them into the LTC editor!');
        note.appendChild(t);

        i = document.createElement('input');
        i.type = 'file';
        i.setAttribute('id', 'upload_files');
        i.setAttribute('placeholder', 'Upload a sketch');
        i.addEventListener('change', uploadSketch, false);
        i.style.width = '0px';
        i.style.height = '0px';
        i.style.border = '0px';
        i.style.margin = '0px';
        i.style.overflow = 'hidden';

        a = document.createElement('a');
        a.innerHTML = 'Upload';
        a.setAttribute('href', '#');
        a.setAttribute('class', 'teal_button');
        a.onclick = function(e) { i.click(); return false; };

        li.appendChild(note);
        li.appendChild(i);
        li.appendChild(a);
        ul.appendChild(li);
    }

    // Add the entire list to the document
    sketchList.appendChild(ul);

    //             <ol>
    //                    <li class='ExampleCategory'>Basics</li>
    //                    <ul>
    //                        <li class='ExampleItem'><a href='examples/01.Basics/AnalogReadSerial/AnalogReadSerial.ino'>Analog Read Serial</a></li>
}

function populateSaveMenu() {
    if (embedMode) {
        return;
    }
    var localSketches = getLocalSketches();
    var li;
    var a;
    var t;
    var padding;
    var note;

    var sketchSaveList = document.getElementById('sketch_save_list');

    // Remove all previous child nodes, in case we're re-populating the list.
    var childNodes = sketchSaveList.childNodes;
    for (var i = 0; i < childNodes.length; i++) {
        sketchSaveList.removeChild(childNodes[i]);
    }

    // Create an unordered list to store the sketch list
    var ul = document.createElement('ul');
    ul.className = 'SketchList';

    // Add sketches stored in localStorage
    for (var name in localSketches) {
        if (!localSketches.hasOwnProperty(name)) {
            continue;
        }
        li = document.createElement('li');
        li.className = 'SketchItem';

        var s = document.createElement('span');
        s.sketchName = name;
        s.innerHTML = name;

        var c = document.createElement('a');
        c.setAttribute('class', 'red_button');
        c.id = 'overwriteSketchName';
        c.value = name;
        c.sketchName = name;
        c.innerHTML = 'Overwrite';
        c.onclick = overwriteSketch;

        var s1 = document.createElement('span');
        s1.innerHTML = ' ';

        li.appendChild(s);
        li.appendChild(s1);
        li.appendChild(c);
        ul.appendChild(li);
    }

    // Add a 'Save As' box
    {
        var lab = document.createElement('label');
        lab.setAttribute('for', 'saveNewSketchName');
        lab.innerHTML = 'Save As: ';

        var tb = document.createElement('input');
        tb.name = 'saveNewSketchName';
        tb.id = 'saveNewSketchName';
        tb.onkeypress = function(e) {
            if (e.keyCode === 13) {
                saveLocalSketchAs(e);
                return false;
            }
            return true;
        };
        tb.type = 'text';

        padding = document.createElement('span');
        padding.innerHTML = ' ';

        a = document.createElement('a');
        a.setAttribute('class', 'teal_button');
        a.setAttribute('href', '#');
        a.innerHTML = 'Save in browser';
        a.onclick = saveLocalSketchAs;

        li = document.createElement('li');
        li.appendChild(lab);
        li.appendChild(padding);
        li.appendChild(tb);
        li.appendChild(padding);
        li.appendChild(a);

        ul.appendChild(li);
    }

    {
        li = document.createElement('li');
        var hr = document.createElement('hr');
        li.appendChild(hr);
        ul.appendChild(li);
    }

    {
        li = document.createElement('li');

        note = document.createElement('p');
        t = document.createTextNode('Files saved in a browser are not permanent.');
        var t2 = document.createTextNode('Please click \'download\' and save your program to your device when you\'re done making changes, so you don\'t lose it!');
        note.appendChild(t);
        note.appendChild(document.createElement('br'));
        note.appendChild(document.createElement('br'));
        note.appendChild(t2);

        i = document.createElement('input');
        i.setAttribute('name', 'download_name');
        i.setAttribute('id', 'download_name');
        i.setAttribute('value', fileName);
        i.onchange = function(t) {
            fileName = t.value;
        };

        padding = document.createElement('span');
        padding.innerHTML = ' ';

        a = document.createElement('a');
        a.innerHTML = 'Download';
        a.setAttribute('href', '#');
        a.setAttribute('class', 'teal_button');
        a.onclick = startDownloadSketch;

        li.appendChild(i);
        li.appendChild(padding);
        li.appendChild(a);
        ul.appendChild(li);
        li.appendChild(note);
    }

    // Add the entire list to the document
    sketchSaveList.appendChild(ul);

    //             <ol>
    //                    <li class='ExampleCategory'>Basics</li>
    //                    <ul>
    //                        <li class='ExampleItem'><a href='examples/01.Basics/AnalogReadSerial/AnalogReadSerial.ino'>Analog Read Serial</a></li>
}

function renderWave(e) {
    var aud = e.target;
    var current = aud.currentTime;
    var end = aud.duration;
    var canvas = getCanvas();

    if (!canvas || !canvas.getContext || !modController || !end) {
        return;
    }

    var strip = canvas.getContext('2d');

    // Resize the canvas to be the window size.
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    var h = strip.canvas.height;
    var w = strip.canvas.width;
    strip.clearRect(0, 0, w, h);

    var y;
    // Draw scale lines at 10% interval
    strip.lineWidth = 1.0;
    strip.strokeStyle = '#55a';
    strip.beginPath();
    y = 1 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 2 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 3 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 4 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 5 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 6 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 7 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 8 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    y = 9 * (h / 10);
    strip.moveTo(0, y);
    strip.lineTo(w, y);
    strip.stroke();

    strip.strokeStyle = '#fff';
    strip.lineWidth = 1.0;

    var buffer = modController.getPcmData();
    var b = Math.floor(buffer.length * ((current * 1.0) / end));
    var lastSample = (buffer[b++] + 32768) / 65536.0; // map -32768..32768 to 0..1

    for (var x = 1; x < canvas.width; x++) {
        var sample = (buffer[b++] + 32768) / 65536.0;
        if (b > buffer.length) {
            break;
        }
        strip.beginPath();
        strip.moveTo(x - 1, h - lastSample * h);
        strip.lineTo(x, h - sample * h);
        strip.stroke();
        lastSample = sample;
    }
}

function installWaveRenderer() {
    var aud = getAudioElement();
    aud.ontimeupdate = renderWave;
}

// Fetch /config.json and update the global "config" variable.
function fetchConfiguration() {
    var request = new window.XMLHttpRequest();
    request.onreadystatechange = function() {
        if ((request.readyState === 4) && (request.status === 200)) {
            config = JSON.parse(request.responseText);
        }
    };
    request.open('GET', '/config.json', true);
    request.send(JSON.stringify(codeobj));
}

function installPiwik() {
    // accurately measure the time spent on the last pageview of a visit
    window._paq.push(['enableHeartBeatTimer']);

    window._paq.push(['disableCookies']);
    window._paq.push(['trackPageView']);
    window._paq.push(['enableLinkTracking']);
    (function() {
        var u = '//ltc-piwik.xobs.io/';
        window._paq.push(['setTrackerUrl', u + 'piwik.php']);
        window._paq.push(['setSiteId', '1']);
        var d = document,
            g = d.createElement('script'),
            s = d.getElementsByTagName('script')[0];
        g.type = 'text/javascript';
        g.async = true;
        g.defer = true;
        g.src = u + 'piwik.js';
        s.parentNode.insertBefore(g, s);
    })();
}

function hasTextChanged() {
    var curtext = editor.getValue();

    if (curtext !== lastText) {
        return true;
    }

    return false;
}

function updateTextChangeBuffer() {
    // stash away the latest snapshot of the text buffer so we can warn of leaving edits
    if (!embedMode) {
        lastText = editor.getValue();
    }
}

function hideMenu() {
    document.getElementById('myDropdown').style.display = 'none';
}

// Look for '#m=embed' in the HTTP fragment, to enable "embed" mode
function initializeEmbed() {
    var fragment = window.location.hash;

    // Strip off the leading '#' that appears
    if (fragment.startsWith('#')) {
        fragment = fragment.split('#', 2)[1];
    }

    var hashElements = fragment.split('&');
    hashElements.forEach(function(element) {
        var arg = element.split('=');
        if ((arg[0] === 'm') && (arg[1] === 'embed')) {
            console.log('Enabling embed mode');
            embedMode = true;
        }
    });

    if (embedMode) {
        window.addEventListener('message', function(event) {
            if (event.data.sourceCode === undefined) {
                return;
            }
            selectTab('code_editor');
            if (hasTextChanged()) {
                var retVal = confirm('Unsaved changes! Proceed and lose changes?');
                if (retVal === false) {
                    return false;
                }
            }
            editor.setValue(event.data.sourceCode);
            editor.refresh();
            resizeHeader();
        });
        document.getElementById('examples_button').style.display = 'none';
        document.getElementById('saveas_button').style.display = 'none';
        document.getElementById('load_button').style.display = 'none';
        document.getElementById('open_button').style.display = '';
        document.getElementById('open_button').addEventListener("click", function(e) {
            var childWindow = window.open("/");
            childWindow.addEventListener("load", function(e) {
                childWindow.postMessage({
                    "sourceCode": editor.getValue()
                }, window.location);
            })
        })
    } else {
        // If we're not in embed mode, listen for 'sourceCode' messages from any parent we may have
        window.addEventListener('message', function(event) {
            if (event.data.sourceCode === undefined) {
                return;
            }
            selectTab('code_editor');
            if (hasTextChanged()) {
                var retVal = confirm('Unsaved changes! Proceed and lose changes?');
                if (retVal === false) {
                    return false;
                }
            }
            editor.setValue(event.data.sourceCode);
            editor.refresh();
            resizeHeader();
        });
    }
}

hideMenu();
fetchConfiguration();
installHooks();
initializeEditor();
initializeEmbed();
resizeHeader();
fixupExamples();
populateSketchList();
populateSaveMenu();
installWaveRenderer();
installPiwik();
updateTextChangeBuffer();