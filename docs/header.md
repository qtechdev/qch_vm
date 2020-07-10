# header
the "header" consists of 16 bytes total, placed at the end of the file.


key           | bytes | values
--------------|-------|-------
magic number  | 2     | `c8` `c8`
version       | 3     | `<major>` `<minor>` `<micro>`
null          | 1     | `00`
display size  | 2     | `<width>` `<height>`
reserved      | 8     | `00` `00` `00` `00` `00` `00` `00` `00`

## example
```
           c8 c8 01 01 00 00 80 40 00 00 00 00 00 00 00 00
          |     |        |  |     |                       |
         /      |        |   \     \                      |
  ______/       |        |    \     \_________            |
 /              /        |     \              \           |
| magic number | version | null | display size | reserved |

```
