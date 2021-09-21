---
layout: default
title: Model.RootNode
description: Returns the first root node in the Model's hierarchy. There may be additional root nodes, and these will be Siblings of this ModelNode. If there are no nodes present on the Model, this will be null.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).RootNode

<div class='signature' markdown='1'>
[ModelNode]({{site.url}}/Pages/Reference/ModelNode.html) RootNode{ get }
</div>

## Description
Returns the first root node in the Model's hierarchy.
There may be additional root nodes, and these will be Siblings
of this ModelNode. If there are no nodes present on the Model,
this will be null.


## Examples

### Non-recursive depth first node traversal
If you need to walk through a Model's node hierarchy, this is a method
of doing this without recursion! You essentially do this by walking the
tree down (Child) and to the right (Sibling), and if neither is present,
then walking back up (Parent) until it can keep going right (Sibling)
and then down (Child) again.
```csharp
static void DepthFirstTraversal(Model model)
{
	ModelNode node  = model.RootNode;
	int       depth = 0;
	while (node != null)
	{
		string tabs = new string(' ', depth*2);
		Log.Info(tabs + node.Name);

		if      (node.Child   != null) { node = node.Child; depth++; }
		else if (node.Sibling != null)   node = node.Sibling;
		else {
			while (node != null)
			{
				if (node.Sibling != null) {
					node = node.Sibling;
					break;
				}
				depth--;
				node = node.Parent;
			}
		}
	}
}
```

