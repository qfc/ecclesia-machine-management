# Machine Master Configuration

This document provides a guide and examples to help setup the machine master
configuration.

## InvertOperation / Devpath Graph Inversion

When merging the devpath graphs from individual agents to construct the machine
/ global devpaths, we may want to change the root node within an individual
agent's devpath graph. This is specified via the InvertOperation proto message
in the config spec. Here is a guide with an example to fill out InvertOperation

For each agent, this message identifies if we need to invert the graph. For eg:
make another node as the new root by inverting all of the links between the
existing root and the new one specified here, in order to make new_root_devpath
the root. When inverting the links, any upstream connectors in the path between
the old root and the new root, will now become downstream connectors. And since
upstream connectors are not encoded into the devpath, we need to specify this
information here. Note that the ordering of the repeated field
upstream_connectors is important. The upstream_connectors need to be from the
old root to the new root. 

For example:

![Example] (https://screenshot.googleplex.com/gCf4obQgGqj.png)

```
invert_ops {
  new_root_devpath: "/phys/A/B/C"
  upstream_connectors: "X"
  upstream_connectors: "Y"
  upstream_connectors: "Z"
}
```
