var editor;
var codeobj = new Object();
var mod_controller;
var autosaveGeneration = null;

function buildResult(results, textStatus, status, jqXHR) {
    if (status !== 200) {
        console.log("Don't know what to do.  Backend failure?");
        return;
    }
    try {
        results = JSON.parse(results);
    }
    catch(e) {
        debugger;
    }
    if (results.success) {
        document.getElementById("buildoutput").innerHTML = "Build complete";

        var data = atob(results.output);
        if (mod_controller)
            mod_controller.stop();
        mod_controller = new ModulationController({
            canvas: document.getElementById("wavStrip"),
            endCallback: function() {
                document.getElementById("Site-footer").style.display = 'none';
            }
        });
        mod_controller.transcodeToAudioTag(data, document.getElementById("audio_output"), 'wav');

        document.getElementById("Site-footer").style.display = '';
    } else {
        editor.chibi_error_string = results.message;
        editor.performLint();
    }
}

function clickUpload(e) {
    selectTab(e);
    document.getElementById("buildoutput").innerHTML = "Building code...";

    // Play empty data onclick to enable audio playback.
    var audioTag = document.getElementById("audio_output");
    var pcmObj = new pcm({
        channels: 1,
        rate: 44100,
        depth: 16
    }).toWav([0, 0]);
    audioTag.src = pcmObj.encode();
    audioTag.play();

    if (mod_controller)
        mod_controller.stop();
    codeobj = {
        "files": [{
            "filename": "project.ino",
            "content": editor.getValue()
        }],
        "format": "binary",
        "version": "167",
        "libraries": [],
        "fqbn": "chibitronics:esplanade:code"
    }

    // Clear out the old error message.
    editor.chibi_error_string = "";
    editor.performLint();

    var request = new window.XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState === 4) {
            buildResult(request.response, request.statusText, request.status, request);
        }
    }
    request.open('POST', "/compile", true);
    request.send(JSON.stringify(codeobj));

    return false;
}

function getLocalSketches() {
        // Get the previous array of sketches
    var localSketches = localStorage.getItem('sketches');
    if (localSketches)
        return JSON.parse(localSketches);
    return {};
}

function saveLocalSketches(localSketches) {
    localStorage.setItem('sketches', JSON.stringify(localSketches));
}

function getGithubToken() {
    return localStorage.getItem("gitHubToken");
}

function saveCurrentEditor() {
    // If this is the first run, autosaveGeneration will be null.
    // If it's not the first run, then we can see whether the
    // document has changed since the last generation.'
    if (autosaveGeneration === null) {
        autosaveGeneration = editor.changeGeneration();
    }
    else if (!editor.isClean(autosaveGeneration)) {
        localStorage.setItem('currentSketch', editor.getValue());
        autosaveGeneration = editor.changeGeneration();
    }

    // Run autosave again in five seconds.
    window.setTimeout(saveCurrentEditor, 5000);
}

function loadSavedEditor() {
    editor.setValue(localStorage.getItem('currentSketch'));
}

function saveLocalSketchAs(e) {
    var localSketches = getLocalSketches();

    // Get the new sketch name
    var sketchName = document.getElementById("saveNewSketchName").value;

    // Store the document in the document list
    localSketches[sketchName] = {
        "name": sketchName,
        "document": editor.getValue()
    };

    // Stash the document list back in local storage
    saveLocalSketches(localSketches);

    // Re-populate the sketch list
    // TODO: Simply add the new sketch instead of redoing everything
    populateSketchList();

    // Don't let the form submit.'
    return false;
}

function initializeEditor() {
    editorNode = document.getElementById("code_editor_textarea");

    var savedSketch = localStorage.getItem('currentSketch');
    if (savedSketch)
        editorNode.innerHTML = savedSketch;

    editor = CodeMirror.fromTextArea(editorNode, {
        value: editorNode.value,
        lineNumbers: true,
        matchBrackets: true,
        mode: "text/x-c++src",
        scrollbarStyle: "null",
        lint: true,
        gutters: ["CodeMirror-lint-markers"],
        useCPP: true
    });
    var mac = CodeMirror.keyMap.default == CodeMirror.keyMap.macDefault;
    CodeMirror.keyMap.default[(mac ? "Cmd" : "Ctrl") + "-Space"] = "autocomplete";

    saveCurrentEditor();
}

function resizeHeader() {
    var header = document.getElementById("Site-header");
    var height = header.offsetHeight;
    document.getElementById("main").style.marginTop = height + 'px';
    header.style.top = 0;
}

function installHooks() {
    window.onresize = resizeHeader;
    document.getElementById("upload_button").onclick = clickUpload;
    document.getElementById("examples_button").onclick = selectTab;
    document.getElementById("saveas_button").onclick = selectTab;
}

function hideShowExampleCategory(e) {
    var elem = e.target;
    if (elem.nextElementSibling.style.display === '')
        elem.nextElementSibling.style.display = 'none';
    else
        elem.nextElementSibling.style.display = '';
    return false;
}

function selectTab(e) {

    var target;
    var elements = document.getElementsByClassName("maintab");

    // If we're called with a string as a parameter, just set that tab
    if ((typeof e) === "string") {
        target = e;
    }
    // Otherwise, treat it as an element, and select that tab.
    else {
        target = e.target.attributes["target"].value;
    }

    var found = false;
    for (var i = 0; i < elements.length; i++) {
        var element = elements[i];

        if (element.id == target) {
            element.style.display = 'block';
            found = true;
        }
        else {
            element.style.display = 'none';
        }
    }

    // If no item was found, make the code editor visible by default
    if (!found) {
        console.log("Warning: Unrecognized element " + target
                    + ", selecting code_editor by default");
        document.getElementById("code_editor").style.display = 'block';
    }
}

function loadExampleFromLink(e) {

    var request = new window.XMLHttpRequest();
    var target = e.target;

    // If we clicked on the <LI> outside of the link, manually select the <A> tag inside.
    if (e.target.tagName === "LI")
        target = target.firstChild;

    request.onreadystatechange = function() {
        if (request.readyState === 4) {

            var editorBox = document.getElementById("code_editor");
            var examplesBox = document.getElementById("examples_list");

            examplesBox.style.display = 'none';
            editorBox.style.display = 'block';

            if (request.status == 200)
                editor.setValue(request.response);
        }
    }
    request.open('GET', target.href, true);
    request.send();

    return false;
}

function fixupExamples() {
    var exampleCategories = document.getElementsByClassName("ExampleCategory");
    for (var i = 0; i < exampleCategories.length; i++) {
        var e = exampleCategories[i];

        // Mark all child categories as hidden
        e.nextElementSibling.style.display = 'none';

        // Set it up so we can click to expand
        e.onclick = hideShowExampleCategory;
    }

    var exampleItems = document.getElementsByClassName("ExampleItem");
    for (var i = 0; i < exampleItems.length; i++) {
        var e = exampleItems[i];

        e.firstChild.onclick = function() { return false; }
        e.onclick = loadExampleFromLink;
    }
}

function undoDeleteLocalSketch(a) {
    var localSketches = getLocalSketches();
    var deletedSketch = localStorage.getItem('deletedSketch');

    // In theory, we can only get here if deletedSketch exists.  If it doesn't,
    // then that's really weird.
    if (deletedSketch === undefined) {
        console.log("No deleted sketch found");
        return false;
    }
    deletedSketch = JSON.parse(deletedSketch);

    // Load the sketch back into the local sketch list, and remove it from "undo"
    localSketches[deletedSketch.name] = deletedSketch;
    localStorage.removeItem('deletedSketch');

    saveLocalSketches(localSketches);

    populateSketchList();

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

    // Add in an "Undo Delete" link
    var li = document.createElement("li");
    li.className = "SketchItem";
    li.innerHTML = "Undo Delete";
    li.onclick = undoDeleteLocalSketch;
    document.getElementById("sketch_list").firstChild.appendChild(li);

    return false;
}

function loadLocalSketch(e) {

    var localSketches = getLocalSketches();
    var sketchName = e.target.sketchName;

    var sketch = localSketches[sketchName];
    if (sketch === undefined) {
        console.log("Couldn't find local sketch " + sketchName);
        return false;
    }

    editor.setValue(sketch.document);
    selectTab("code_editor");

    return false;
}

function populateSketchList() {
    var localSketches = getLocalSketches();

    var sketchList = document.getElementById("sketch_list");

    // Remove all previous child nodes, in case we're re-populating the list.
    var child_nodes = sketchList.childNodes;
    for (var i = 0; i < child_nodes.length; i++)
        sketchList.removeChild(child_nodes[i]);

    // Create an unordered list to store the sketch list
    var ul = document.createElement("ul");
    ul.className = "SketchList";

    // Add an entry for GitHub, or add a signup link if one doesn't exists
    if (getGithubToken() == null) {
        var li = document.createElement("li");

        var a = document.createElement("a");
        a.innerHTML = "Connect to GitHub";
        a.setAttribute("href", "/connectToGitHub");
        a.setAttribute("target", "__new");

        li.appendChild(a);
        ul.appendChild(li);
    }
    else {
        ;
    }

    // Add sketches stored in localStorage
    for (name in localSketches) {
        if (!localSketches.hasOwnProperty(name))
            continue;
        var li = document.createElement("li");
        li.className = "SketchItem";

        var s = document.createElement("span");
        s.sketchName = name;
        s.onclick = loadLocalSketch;
        s.innerHTML = name;

        var a = document.createElement("a");
        a.setAttribute("href", "#deleteLocalSketch");
        a.className = "SketchItemDelete";
        a.sketchName = name;
        a.onclick = deleteLocalSketch;
        a.innerHTML = "[X]";

        li.appendChild(s);
        li.appendChild(a);
        ul.appendChild(li);
    }

    // Add a "Save As" box
    {
        var lab = document.createElement("label");
        lab.setAttribute("for", "saveNewSketchName");
        lab.innerHTML = "Save As:";

        var tb = document.createElement("input");
        tb.name = "saveNewSketchName";
        tb.id = "saveNewSketchName";
        tb.type = "text";

        var sub = document.createElement("input");
        sub.type = "submit";
        sub.onclick = saveLocalSketchAs;

        var li = document.createElement("li");
        li.appendChild(lab);
        li.appendChild(tb);
        li.appendChild(sub);

        ul.appendChild(li);

    }

    // Add the entire list to the document
    sketchList.appendChild(ul);

//             <ol>
//                    <li class="ExampleCategory">Basics</li>
//                    <ul>
//                        <li class="ExampleItem"><a href="examples/01.Basics/AnalogReadSerial/AnalogReadSerial.ino">Analog Read Serial</a></li>
}

installHooks();
initializeEditor();
resizeHeader();
fixupExamples();
populateSketchList();
