This folder contains documentation for developers only.
For user documentation, refer to our [website](https://www.precice.org/docs.html).


### 🛠️ Nearest-Projection Advanced Configuration

The `nearest-projection` mapping now supports advanced search parameters to handle complex geometries like thin meshes.

| Attribute | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| **`n-nearest`** | `int` | `4` | Number of candidate primitives to fetch from the spatial index. |
| **`full-search`** | `bool` | `false` | If `true`, evaluates all candidates to find the absolute minimum distance. |

> **Pro Tip:** For thin-shell structures, setting `full-search="true"` ensures the mapping finds the mathematically closest triangle, even if the index tree returns a triangle from the "wrong" side first.

💡 When to Use These Settings
Handling "Thin Meshes"
In scenarios where two surfaces are physically very close to each other (e.g., a thin membrane or a shell structure), the default "short-circuit" logic might incorrectly map a vertex to the "wrong" side of the shell.

Problem: The R-Tree spatial index might return a triangle from the far side of the membrane first due to the internal ordering of the tree.

Solution: Set full-search="true" and increase n-nearest (e.g., to 8 or 12). This forces preCICE to check multiple candidates and mathematically ensure the absolute closest projection is selected, rather than stopping at the first valid one found.

Example Configuration
<mapping:nearest-projection 
    direction="write" 
    from="MeshA" 
    to="MeshB" 
    constraint="consistent"
    n-nearest="8"
    full-search="true" />




