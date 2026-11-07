# SMP Tool

`smp` generates SMP XML artifacts from XSMP models.

## When to use it

Use the SMP tool when you want XSMP sources to produce standard SMP artifacts such as:

- catalogue XML
- package XML
- configuration XML
- assembly XML
- link base XML
- schedule XML

## How to enable it

Add this to `xsmp.project`:

```text
tool 'smp'
```

## What it adds

The tool contributes generation of SMP XML artifacts under:

```text
smdl-gen/
```

Typical outputs include files such as:

- `.smpcat`
- `.smppkg`
- `.smpcfg`
- `.smpasb`
- `.smplnk`
- `.smpsed`

depending on which XSMP documents exist in the project.

## Typical project snippet

```text
project 'orbitalsegment'
source 'smdl'
tool 'smp'
```

## Expected result

After a successful generation, `smdl-gen/` contains SMP XML files that correspond to the XSMP documents found in the project sources.

## Characters that XML cannot hold

Any character is a valid value in XSMP, but XML 1.0 cannot hold most control characters, U+FFFE and U+FFFF, not even as `&#0;`. The SMP tool writes each of their bytes as `\xHH`, and reading the file back gives the same value:

| XSMP value | In the XML |
| --- | --- |
| `'\0'` | `\x00` |
| `"a\x01b"` | `a\x01b` |
| `"\uFFFE"` | `\xEF\xBF\xBE` |
| `"C:\\dir"` | `C:\dir` |
| `"\\x01"` | `\x5Cx01` |

Any other text stays as it is, backslashes included. Only a backslash that would read as such an escape is written `\x5C`.

## `nullptr`

SMP XML has no form for `nullptr`, so such a value is left out. In ECSS SMP 2025, the default of a parameter is C++ text and keeps its `nullptr`.
