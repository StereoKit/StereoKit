---
---

var search = "";

function updateSearch(text) {
    search = text.toLowerCase();
    
    // Check each leaf to see if it's visible
    var leaves = document.getElementsByClassName('tree-leaf');
    for(var i=0; i<leaves.length; i+=1) {
        var valid = nameValid(leaves[i].textContent);
        leaves[i].style.display = valid ? "block" : "none"; 
    }

    // See if a branch has any visible leaves
    var branches = document.getElementsByClassName('tree-branch');
    for(var i=0; i<branches.length; i+=1) {
        var leaves = branches[i].getElementsByClassName('tree-leaf');
        var valid  = false;
        for (var f=0; f<leaves.length; f+=1) {
            if (leaves[f].style.display === "block") {
                valid = true;
            }
        } 
        branches[i].style.display = valid ? "block" : "none"; 
    }
}
function nameValid(name) {
    return search === "" || name.toLowerCase().includes(search);
    
}
function renderFolder(folder, parent) {
    var result = parent == "" ? "<ul>" : "<li class='tree-branch'><input type='checkbox' "+(parent=="Pages"?"":"checked")+" /><span></span><a href='{{site.url}}/"+parent+"/"+folder.name.replace(" ", "-")+".html'>"+folder.name+"</a><ul>";
    if (parent != "") parent += "/";
    parent += folder.name.replace(" ", "-");

    // Render the pages in this folder
    if (folder.pages) {
        for (var i = 0; i<folder.pages.length; i+=1) {
            result += "<li class='tree-leaf'><a href='{{site.url}}/"+parent+"/"+folder.pages[i].replace(" ", "-")+".html'>"+folder.pages[i]+"</a></li>";
        }
    }

    // Render sub-folders
    if (folder.folders) {
        for (var i = 0; i<folder.folders.length; i+=1) {
            var folderText = renderFolder(folder.folders[i], parent);
            result += folderText;
        }
    }
    result += "</ul></li>";
    return result;
}
function renderNav() {
    var nav = document.getElementById("links");
    nav.innerHTML = renderFolder(documents, "");
}

window.addEventListener('load', function () {
    renderNav();
});