# DawBot native UI implementation

Source of truth: [approved Figma screens](https://www.figma.com/design/lRVGvZEVGUkl50HOi49IOb?node-id=3-2).

## Scope

Implement the first usable native JUCE UI increment: home, live chat, project library/detail,
learning library/detail, tutorial progress, history, DAW preferences and tools settings.
Keep the existing plugin/helper boundary, screen guidance and MIDI drag-out. Use the current
Figma logo, charcoal palette and #FA684A; stock preferences live only in Settings. No mobile teaser.

Native components fit this repository's JUCE 8/C++20 architecture. A webview would introduce
a second UI runtime and bridge without improving the existing native integrations.

## Implementation plan

- [x] Add native regression tests for state round-trips, offline draft retention and navigation.
- [x] Introduce bounded, versioned workspace state for preferences, projects, saved conversations,
  history and lesson progress. Persist it in the host's plugin state, not a new cloud service.
- [x] Integrate preferences with the existing helper request context. Reject stale replies and
  clear request state on new chat. Preserve live step navigation and generated MIDI clips.
- [x] Build reusable native controls and responsive layouts using the Figma assets and tokens.
  Implement actual navigation, project CRUD, search, settings and learning interactions.
- [x] Add a file picker/drop target for local references. Clearly mark audio analysis as not yet
  connected; do not pretend the current text/vision helper listened to the attachment.
- [x] Build and run native tests, render desktop/compact screenshots, review them against Figma,
  then open a draft pull request with the implementation and remaining backend work.

## Verification focus

Offline submission must preserve the draft. New chat must invalidate in-flight responses.
Long content must scroll at compact widths. Restored state must validate types and limits.
Disabling history must omit unsaved conversations from persisted state. No network or file I/O
may be added to the audio callback. File contents must never be uploaded by the new picker.

## Boundaries

Learning content is bundled starter material, not a remote CMS. Projects are scoped to the
plugin instance and saved with the host session. Audio analysis, mobile/cloud sync, billing,
and a production authentication flow remain later work. API keys remain in the existing helper.


## Verified on 2026-11-16

- macOS Apple silicon, JUCE 8.0.4, CMake 3.31.10, Apple Command Line Tools.
- Debug builds: shared plugin, VST3, AU, Standalone and DawBot Helper all succeeded. Builds were not installed in the user's DAW plugin directories.
- Native regression run: 104 checks passed, zero failures and no JUCE assertions. This includes real localhost IPC against a deterministic helper double; no external model calls or credentials were used.
- Regression fixes cover unsaved forms across resize, settings refresh during host restore, queued stale replies during restore, and project budget enforcement for create/rename/save.
- Native captures reviewed at 1200 x 1000 and 800 x 760. Content scrolls at compact sizes; the chat composer stays visible. Original Figma assets render from bundled resources at their intended header/card dimensions.
- Inter and its license are bundled. The brand uses lowercase dawbot, matching the updated design file. Articulat CF is not bundled.

### Captures

These are native component snapshots from the test application. Chat response text is a deterministic test fixture, not a live AI answer.

| Screen | Native capture |
| --- | --- |
| Home | [1200 px](screenshots/home-1200.png) / [800 px](screenshots/home-800.png) |
| Chat | [Answer and fixed composer](screenshots/chat-answer.png) |
| Projects | [Saved guide in project](screenshots/project-detail.png) |
| Guides | [Guide library](screenshots/guides.png) |
| Tutorial | [Lesson and progress](screenshots/tutorial-step.png) |
| Settings | [My tools](screenshots/settings-tools.png) |

### Remaining validation and work

This is a first native UI increment, not a release of every future Figma screen. Before shipping, test the plugin in Logic/Ableton, MIDI drag-out, screen permissions, actual model replies, accessibility with VoiceOver, and Intel/Windows builds. A production content catalog, file/audio analysis, authentication/billing and mobile/cloud sync are not implemented. Project/history data stays scoped to the plugin instance in the saved DAW session.
