# DawBot UI assets

Source design: [DawBot Plugin UI / Design System](https://www.figma.com/design/lRVGvZEVGUkl50HOi49IOb?node-id=3-2).

- `logo.png`: transparent 3x export of the supplied logo component `6:199`, including the supplied SVG child `24:19`. Rendered in the native header at 42 x 42 and home welcome at 92 x 92 (64 x 64 compact).
- `*.svg`: original icon exports referenced by the Figma home and library screen design contexts. Native controls render them in the 24 px header / 28 px card slots. These are local build resources; no temporary Figma asset URLs are used at runtime.
- `InterRegular.ttf`, `InterSemiBold.ttf`: static instances of [Google Fonts' Inter variable font](https://github.com/google/fonts/tree/main/ofl/inter), optical size 14, weights 400 and 600. The fontTools instancer generated the static files and set the corresponding family/style metadata. Inter is the available typeface used in the current Figma screens. Articulat CF is not bundled.
- `Inter-OFL.txt`: original SIL Open Font License 1.1 and attribution, embedded alongside the fonts and accessible through Settings > Help > Copy font license.

The palette lives in `Source/Plugin/UI/Components.h`; the orange is exactly `#FA684A`.
