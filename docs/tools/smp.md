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

## String template values and the published XSD

XSMP keeps string template values as they are, such as a schedule root path:

```xsmp
schedule <Root = "/Root"> Demo
```

gives this parameter in the schedule XML:

```xml
<Parameter xsi:type="Assembly:StringArgument" Name="Root" Value="/Root"/>
```

The published SMP Level 2 XSD, `xml/Smdl/Assembly.xsd` in
`ECSS_SMP_L2_(31March2025).zip`, gives `StringArgument/@Value` the type
`Assembly:Name`, with this pattern:

```text
(([a-zA-Z0-9_])|\{[a-zA-Z0-9_]*\})*
```

It takes ASCII letters, digits, `_` and `{...}` template tokens, but not `/` or
`.`, so a validator of that XSD rejects `/Root`. Yet
[ECSS-E-ST-40-08C](https://ecss.nl/standard/ecss-e-st-40-08c-simulation-modelling-platform-level-2-5-august-2025/),
clauses 4.2.4.4 and 5.4.3.2b, describes a String8 template argument that gives
the schedule root path, and such a path separates component names with `/`.

The same XML type holds the string defaults of assemblies and schedules, and the
explicit arguments of assembly instances and task executions. XSMP keeps the
value and reports nothing for this restriction, since removing a separator would
change the path. A tool that enforces the published XSD can reject these files.

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
