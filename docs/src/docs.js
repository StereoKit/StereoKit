---
---

var search = "";

var treeDict = { data:{} };
treeDict.hasItem = function (name) {
    return treeDict.data[name] > 0;
}
treeDict.setItem = function(name, val) {
    if (val)
        treeDict.data[name] = 1;
    else
        delete treeDict.data[name];
        
    sessionStorage.setItem("treeDict", Object.keys(treeDict.data).join());
}
treeDict.load = function() {
    treeDict.data = {};
    var treeData  = sessionStorage.getItem("treeDict");
    var treeItems = treeData == null ? [] : treeData.split(",");
    for (var i=0; i<treeItems.length; i++) {
        treeDict.data[treeItems[i]] = 1;
    }

    if (treeData == null) {
        treeDict.data["Guides"] = 1;
        treeDict.data["Reference"] = 1;
    }
}

function updateVisibility(folder) {
    var valid = nameValid(folder.name);
    
    if (folder.pages) {
        for (var i = 0; i<folder.pages.length; i+=1) {
            valid = updateVisibility(folder.pages[i]) || valid;
        }
    }

    folder.element.style.display = valid ? "block" : "none";
    if (folder.check)
        folder.check.checked = !(search !== "" || treeDict.hasItem(folder.name));
    return valid;
}

function updateSearch(text) {
    sessionStorage.setItem("search", text);
    search = text.toLowerCase();
    
    for (var i = 0; i<documents.pages.length; i+=1) {
        updateVisibility(documents.pages[i]);
    }
}

function nameValid(name) {
    return search === "" || name.toLowerCase().includes(search);
}

function renderFolder(folder, parent, hierarchy) {
    // Check if it has children
    var hasChildren = folder.pages != null && folder.pages.length > 0;

    // Add the root DOM node for the folder
    var folderItem = document.createElement("li");
    folderItem.className = hasChildren ? "tree-branch" : "tree-leaf";
    folder.element = folderItem;
    parent.appendChild(folderItem);

    var expanderContainer = document.createElement("span");
    expanderContainer.className = "expander-container";
    expanderContainer.setAttribute("aria-hidden", true);

    // If there are child pages, make it expandable
    if (hasChildren) {
        var expanderId = "expander-" + folder.name.replace(/ /g, '-');

        // Label which proxies clicks to the expander checkbox input
        var expander = document.createElement("label");
        expander.htmlFor = expanderId;
        expanderContainer.appendChild(expander);

        // The checkbox input used to store expand state. Also used in CSS to hide the child page lists and for keyboard focus.
        var checkbox = document.createElement("input");
        checkbox.id = expanderId;
        checkbox.className = "tree-branch-check";
        checkbox.type      = "checkbox";
        checkbox.setAttribute('aria-label', "Expand " + folder.name);
        checkbox.onclick   = function() { treeDict.setItem(folder.name, !this.checked); };
        folder.check = checkbox;
        folderItem.appendChild(checkbox);
    }

    // Append the expander container after the checkbox input
    folderItem.appendChild(expanderContainer);

    // Link to the actual document
    var link = document.createElement("a");
    link.href      = "{{site.url}}/"+hierarchy+"/"+folder.name.replace(/ /g, '-')+".html";
    link.innerText = folder.name;
    folderItem.appendChild(link);

    // Render sub-folders
    if (hasChildren) {
        // Update the URL hierarchy
        hierarchy += "/"+folder.name.replace(/ /g, '-');

        // We need a sub-menu for the folding to work
        var subList = document.createElement("ul");
        folderItem.appendChild(subList);

        for (var i = 0; i<folder.pages.length; i+=1) {
            renderFolder(folder.pages[i], subList, hierarchy);
        }
    }
}

function renderNav() {
    var nav  = document.getElementById("links");
    var list = document.createElement("ul");
    
    for (var i = 0; i<documents.pages.length; i+=1) {
        renderFolder(documents.pages[i], list, "Pages");
    }
    nav.appendChild(list);
}

window.addEventListener("load", function () {
    renderNav();

    treeDict.load();
    
    var searchBar = document.getElementById("search");
    searchBar.value = sessionStorage.getItem("search");
    updateSearch(searchBar.value);
});