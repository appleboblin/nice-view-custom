# nice-view-custom

A nice!view status screen for ZMK **main / v0.4 (Zephyr 4.1, LVGL 9)**, for split keyboards.

![Preview](art/preview.png)

*Left to right: central connected on Graphite; charging, profile 2 not connected, NumRow; USB, Function; peripheral. Rendered from the module's drawing code with LVGL 9.3.*

| Section | Central (left) | Peripheral (right) |
|---|---|---|
| Top band (24 px) | Battery icon, connection icon | Battery icon, link icon |
| Art (112 px) | 68×112 image | 68×112 image |
| Bottom band (24 px) | BLE profile dots, active layer name | Optional label |

Connection icon: USB symbol = USB output; Bluetooth + ✓ connected; Bluetooth + ⟳ pairing (empty profile, advertising); Bluetooth + ✕ paired device not connected. On the peripheral the icon shows its link to the central.

Battery: each half shows its own; the bar shows the level, a bolt replaces it while charging over USB.

Profile dots: filled = active, ring = paired, small dot = empty profile.

## Usage

`config/west.yml`:

```yaml
manifest:
  remotes:
    - name: appleboblin
      url-base: https://github.com/appleboblin
  projects:
    - name: nice-view-custom
      remote: appleboblin
      revision: main
```

`build.yaml` — use `nice_view_custom` instead of `nice_view`:

```yaml
include:
  - board: nice_nano//zmk
    shield: corne_left nice_view_adapter nice_view_custom
  - board: nice_nano//zmk
    shield: corne_right nice_view_adapter nice_view_custom
```

Requires ZMK `main` (or v0.4 once released). It will not build on v0.3 (LVGL 8).

## Options (`<keyboard>.conf`)

| Option | Default | |
|---|---|---|
| `CONFIG_NICE_VIEW_CUSTOM_ART` | `""` | Path to your art `.c` file, absolute or relative to the `config/` folder. Empty = built-in placeholder. |
| `CONFIG_NICE_VIEW_CUSTOM_INVERTED` | `y` | White on black. Set `n` for black on white |
| `CONFIG_NICE_VIEW_CUSTOM_PERIPHERAL_LABEL` | `""` | Text in the peripheral's bottom band, e.g. `"Corne"` |

Layer names come from each layer's `display-name` (the name in `ZMK_LAYER(name, ...)` with zmk-helpers). Layers without one show `Layer N`.

## Custom art

The art slot is 68 wide × 112 tall, 1-bit. Draw it upright (portrait); the converter rotates it for the panel.

```sh
pip install pillow
python3 tools/png2lvgl.py my_art.png -o config/art.c --preview preview.png
```

then in `config/corne.conf`:

```
CONFIG_NICE_VIEW_CUSTOM_ART="art.c"
```

Converter flags: `--dither` (photos), `--threshold N` (0–255, default 128), `--invert` (swap ink/background in the source), `--fit contain|cover|none`. Images larger than 68×112 are shrunk; smaller ones stay pixel-exact and are centered. Dark pixels in your source image become ink (drawn white on the default black screen); light and transparent pixels become background. Pixel art drawn at exactly 68×112 looks best on this panel.

The art file is compiled into both halves. Only use artwork you have the rights to.

## Credits

Drawing helpers and display setup adapted from ZMK's built-in nice!view shield (MIT). Layout idea inspired by [infely/nice-view-battery](https://github.com/infely/nice-view-battery).
