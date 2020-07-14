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

    // If there are child pages, make it expandable
    if (hasChildren) {
        // The expand tree arrow
        var expander = document.createElement("input");
        expander.className = "tree-branch-check";
        expander.type      = "checkbox";
        expander.setAttribute('aria-label', "Expand " + folder.name);
        expander.onclick   = function() { treeDict.setItem(folder.name, !this.checked); };
        folder.check = expander;
        folderItem.appendChild(expander);

        // A span for some of the tree trickery
        folderItem.appendChild(document.createElement("span"));
    }

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