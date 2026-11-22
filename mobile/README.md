# dawbot mobile (Expo / React Native)

The phone half of dawbot: **learn and sketch on the go**. The plugin (`../Source`) is the AI agent
inside the DAW; this app teaches with short lessons, captures ideas, and hands projects over to
the plugin ("Send to studio").

Built with Expo SDK 58, Expo Router, TypeScript, `lucide-react-native` icons, and Inter / DM Sans fonts.
Screens match the design canvas "dawbot mobile app — screens" one for one.

## Run it

```bash
cd mobile
npm install
npx expo start          # press i (iOS simulator), a (Android) or scan the QR code with Expo Go
npm run web             # or try it in a browser
```

Checks:

```bash
npm run typecheck       # tsc --noEmit
npm run lint            # expo lint
```

## Screens and routes

| Flow | Route | File |
| --- | --- | --- |
| Welcome | `/onboarding` | `src/app/onboarding/index.tsx` |
| Pick your DAW · level · goals | `/onboarding/daw`, `/level`, `/goals` | `src/app/onboarding/*.tsx` |
| Create account · link your studio | `/onboarding/account`, `/link` | `src/app/onboarding/*.tsx` |
| Learn path (tab) | `/learn` | `src/app/(tabs)/learn.tsx` |
| Lesson start sheet | `/lesson/start` | transparent modal |
| Lessons: build the chord · ear training · put steps in order | `/lesson/chord`, `/listen`, `/order` | with right / wrong result trays |
| Lesson complete | `/lesson/complete` | awards XP, offers "Send to studio" |
| Streak & achievements · weekly league | `/streak`, `/league` | |
| Ideas (tab) | `/ideas` | `src/app/(tabs)/ideas.tsx` |
| Record a hum · hum to MIDI · chord sketch · beat sketch | `/ideas/record`, `/hum`, `/chords`, `/beat` | |
| Projects (tab) · project detail · new project | `/projects`, `/projects/[id]`, `/projects/new` | |
| Send to studio · saved answer from the plugin | `/projects/send`, `/projects/saved` | |
| Ask (tab) | `/ask` | theory and sound questions; DAW-specific help lives in the plugin |
| Profile (tab) · settings | `/profile`, `/settings` | |

## Project layout

```
src/app/            routes (Expo Router); _layout.tsx loads fonts and the store
src/components/     ui.tsx (buttons, cards, chips, headers, piano, waveform), lesson.tsx, ideas.tsx
src/data/           lesson content
src/store.tsx       in-memory app state (DAW, goals, streak, XP, ideas, projects)
src/theme.ts        colors, fonts, type scale (shared with the plugin's look)
assets/images/      robot renders, logo, DAW icons
```

## What's mocked (next steps)

Search the code for `TODO(` to find each one.

- **Accounts:** Sign in with Apple / Google / email (`onboarding/account.tsx`).
- **Plugin sync:** pairing code, "Send to studio" and project sync need endpoints on the dawbot API
  (`server/`), which the plugin polls on launch.
- **Audio:** recording, playback of lesson loops and sketches (`expo-audio`), and hum-to-MIDI on the
  server.
- **Ask:** wire to `server/coach.mjs` with a mobile mode (no screen capture, no DAW actions).
- **Persistence:** the store is in memory; persist with `expo-sqlite` or the API.
- Leaderboard names are placeholders.

## Ship it

Use EAS (`npx eas-cli@latest build`, `submit`) to build and send to the App Store and Google Play.
Bundle IDs are `ai.dawbot.app` on both platforms (`app.json`).
