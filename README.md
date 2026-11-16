# DawBot.ai — Phase 1 (JUCE)

An AI production coach that lives in your DAW. Ask a question and DawBot looks at
your screen, **highlights exactly where to click**, walks you through it step by step,
generates **MIDI clips you drag straight onto a track**, and builds **plugin chains from
the plugins you actually own** (Serum, Valhalla, stock devices...).

```
┌──────────────── Your DAW (Ableton / Logic / FL / Cubase …) ───────────────┐
│  DawBot plugin (VST3 / AU)                                                 │
│   • chat window             • reads host name, tempo, time signature      │
│   • step-by-step navigator  • drag-out MIDI clip tile                      │
└──────────────┬─────────────────────────────────────────────────────────────┘
               │ localhost socket (127.0.0.1:52817, JSON)
┌──────────────▼─────────────── DawBot Helper (desktop app) ────────────────┐
│  • screen capture (ScreenCaptureKit on macOS, GDI on Windows)             │
│  • installed-plugin scan (file names only, never loads plugins)           │
│  • AI client: Claude or OpenAI, vision + structured JSON output           │
│  • transparent click-through overlay: pulsing highlight + step callout    │
└──────────────┬─────────────────────────────────────────────────────────────┘
               │ HTTPS
       Anthropic / OpenAI  (dev)   or   api.dawbot.ai proxy (production, see /server)
```

### Why a plugin + helper app, not just a plugin?
1. **Drawing over the DAW.** A plugin window can't draw on top of the DAW's arrangement view or
   piano roll. The helper owns a transparent, click-through, always-on-top overlay window.
2. **Permissions.** macOS grants Screen Recording per app. With a helper, the user approves
   *DawBot Helper* once instead of approving Ableton, Logic, FL, and so on separately. Logic also runs AUs
   out-of-process on Apple silicon, where capture permission inside the plugin gets messy.
3. **Stability.** Network calls and image encoding stay out of the DAW's process.

## Project layout
```
CMakeLists.txt              builds both targets; fetches JUCE 8 automatically
Source/Shared/              Protocol.h (IPC messages), MidiClip (AI notes -> .mid)
Source/Plugin/              PluginProcessor, PluginEditor, HelperConnection, MidiDragTile
Source/Helper/              Main (app + settings window), HelperServer (IPC),
                            AIClient + CoachPrompt (system prompt + JSON schema),
                            ScreenCapture_{mac.mm,win.cpp,stub.cpp}, Overlay, PluginScanner, Config
server/proxy.mjs            dev API proxy: holds your API keys, checks user tokens, meters usage
```

## Build
Requirements: CMake ≥ 3.22, plus **Xcode 15+** (macOS) or **Visual Studio 2022** (Windows).
Screen capture needs **macOS 14+**; everything else runs on macOS 11+.

```bash
# macOS
cmake -B build -G Xcode
cmake --build build --config Release

# Windows (Developer PowerShell)
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```
The plugin is copied to your plugin folders automatically (`COPY_PLUGIN_AFTER_BUILD`).
The helper is at `build/DawBotHelper_artefacts/Release/`. Move it to `/Applications`
(macOS) or `C:\Program Files\DawBot\` (Windows) so the plugin's **Launch Helper** button can find it.

Validate the AU for Logic: `auval -v aufx Dbt1 Dwbt`

## First run
1. Open **DawBot Helper**, choose Claude or OpenAI, paste an API key, then click **Save**
   (or set `ANTHROPIC_API_KEY` / `OPENAI_API_KEY` in the environment).
2. macOS: click **Grant Screen Recording**, enable DawBot Helper, and relaunch it.
   *Dev tip:* code-sign your builds with a stable identity. Unsigned rebuilds look like a new app
   to macOS, which then asks for permission again.
3. In your DAW, put **DawBot** on any track (it passes audio through untouched) and open it.
4. Ask something like: *"Show me how to draw an E minor chord progression"*. Then use ◀ ▶ to move through
   the highlighted steps and drag the MIDI tile onto a MIDI track.

## How a question flows
1. The plugin sends `{question, host, bpm, timeSignature}` to the helper.
2. The helper captures the display under the mouse (excluding its own overlay), downsizes it to
   1280 px, and JPEG-encodes it.
3. The model receives the screenshot, the session context, the installed-plugin list and the last few turns.
   Output is forced into a schema: `answer`, `steps[{instruction, target box}]`, `midi`, `plugin_chain`.
   - Claude: a forced tool call (`tool_choice`) with the schema as `input_schema`.
   - OpenAI: `response_format: json_schema` with `strict: true`.
4. Target boxes are mapped from screenshot pixels to screen coordinates. The overlay then highlights
   step 1, and the plugin shows the answer, the step navigator and the MIDI tile.

## Known limits (Phase 1)
- **Highlight accuracy** depends on the model's visual grounding. Big, clearly labelled controls work
  well; tiny icons may be off by a few pixels. The prompt tells the model to return `null`
  rather than guess. Phase 1.5: add per-DAW UI maps and a "Was this the right spot?" correction loop.
- The overlay doesn't appear over a DAW in macOS **full-screen mode** (a separate Space). Use a normal window.
- Pro Tools needs **AAX**: add `AAX` to `FORMATS` after you join the Avid developer program
  and set up PACE signing.
- API keys are stored in plain text in `config.json` for dev builds. For release, use the proxy
  (`server/proxy.mjs` → your real backend) so only a DawBot token lives on the user's machine.
- The plugin doesn't save chat history in the DAW project yet.

## Production proxy (dev version included)
```bash
ANTHROPIC_API_KEY=sk-ant-... DAWBOT_DEV_TOKENS=devtoken123 node server/proxy.mjs
```
Then set `"baseUrl": "http://localhost:8787"` and `"authToken": "devtoken123"` in the helper's
`config.json` (macOS: `~/Library/Application Support/DawBot/`, Windows: `%APPDATA%\DawBot\`).
This proxy is where subscriptions, trials and credit packs get enforced.
