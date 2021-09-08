## SVG validation plugin

This plugin validates SVG images as required by the BIMI specification. It depends on libxml2 and is based on RelaxNG.
It add the ``bimi_svg_check`` function to HSL.

### smtpd.yaml

```
plugins:
  - id: bimi
    path: /opt/halon/plugins/bimi.so
    config:
      schema: /path/to/SVG_1.2_PS.rng
```
