import { createContext, useContext, useMemo, useState, type ReactNode } from 'react';
import type { DawId } from './assets';

export type Level = 'new' | 'starting' | 'tracks' | 'releasing';
export type Topic = 'beats' | 'chords' | 'vocals' | 'sound' | 'mixing' | 'arrangement' | 'theory' | 'daw';
export type IdeaKind = 'hum' | 'chords' | 'beat' | 'note';

export type Idea = {
  id: string;
  kind: IdeaKind;
  title: string;
  meta: string;
  projectId: string | null;
  section?: string;
};

export type Project = {
  id: string;
  name: string;
  daw: string;
  bpm: number;
  key: string;
  status: 'synced' | 'phone' | 'studio';
  sections: { name: string; sketched: boolean }[];
};

type State = {
  daw: DawId;
  level: Level;
  topics: Topic[];
  dailyMinutes: number;
  streak: number;
  xp: number;
  minutesToday: number;
  ideas: Idea[];
  projects: Project[];
};

type Actions = {
  setDaw: (d: DawId) => void;
  setLevel: (l: Level) => void;
  toggleTopic: (t: Topic) => void;
  setDailyMinutes: (m: number) => void;
  completeLesson: (xp: number, minutes: number) => void;
  addIdea: (idea: Omit<Idea, 'id'>) => void;
  addProject: (p: Omit<Project, 'id' | 'status'>) => string;
  sendToStudio: (projectId: string) => void;
};

const StoreContext = createContext<(State & Actions) | null>(null);

const SONG_SECTIONS = ['Intro', 'Verse', 'Hook', 'Verse 2', 'Bridge', 'Outro'];

const initialState: State = {
  daw: 'ableton',
  level: 'tracks',
  topics: ['beats', 'chords', 'sound', 'daw'],
  dailyMinutes: 10,
  streak: 13,
  xp: 340,
  minutesToday: 6,
  ideas: [
    { id: 'i1', kind: 'hum', title: 'Hummed hook', meta: 'Today · 0:14 · MIDI ready, F minor', projectId: 'night-drive', section: 'Hook' },
    { id: 'i2', kind: 'chords', title: 'Chord loop', meta: 'Today · Fm D♭ A♭ E♭ · 92 BPM', projectId: 'night-drive', section: 'Verse' },
    { id: 'i3', kind: 'beat', title: 'Beat sketch', meta: 'Yesterday · 124 BPM · 2 bars', projectId: 'summer-flip', section: 'Intro' },
    { id: 'i4', kind: 'note', title: 'Lyric note', meta: 'Mon · headlights in the rain', projectId: null },
  ],
  projects: [
    { id: 'night-drive', name: 'Night Drive', daw: 'Ableton Live 12', bpm: 92, key: 'F minor', status: 'synced', sections: SONG_SECTIONS.map((n, i) => ({ name: n, sketched: i < 3 })) },
    { id: 'summer-flip', name: 'Summer flip', daw: 'Ableton Live 12', bpm: 124, key: 'A minor', status: 'phone', sections: ['Intro', 'Build', 'Drop', 'Break', 'Outro'].map((n, i) => ({ name: n, sketched: i < 1 })) },
    { id: 'sunday-samples', name: 'Sunday Samples', daw: 'Logic Pro', bpm: 88, key: 'D minor', status: 'studio', sections: ['Intro', 'Verse', 'Hook', 'Verse 2', 'Outro'].map((n, i) => ({ name: n, sketched: i < 4 })) },
  ],
};

let nextId = 100;

export function StoreProvider({ children }: { children: ReactNode }) {
  const [state, setState] = useState<State>(initialState);

  const value = useMemo<State & Actions>(
    () => ({
      ...state,
      setDaw: (daw) => setState((s) => ({ ...s, daw })),
      setLevel: (level) => setState((s) => ({ ...s, level })),
      toggleTopic: (t) =>
        setState((s) => ({ ...s, topics: s.topics.includes(t) ? s.topics.filter((x) => x !== t) : [...s.topics, t] })),
      setDailyMinutes: (dailyMinutes) => setState((s) => ({ ...s, dailyMinutes })),
      completeLesson: (xp, minutes) =>
        setState((s) => ({ ...s, xp: s.xp + xp, minutesToday: s.minutesToday + minutes, streak: s.minutesToday === 0 ? s.streak + 1 : s.streak })),
      addIdea: (idea) => setState((s) => ({ ...s, ideas: [{ ...idea, id: `i${nextId++}` }, ...s.ideas] })),
      addProject: (p) => {
        const id = `${p.name.toLowerCase().replace(/[^a-z0-9]+/g, '-')}-${nextId++}`;
        setState((s) => ({ ...s, projects: [{ ...p, id, status: 'phone' }, ...s.projects] }));
        return id;
      },
      sendToStudio: (projectId) =>
        setState((s) => ({ ...s, projects: s.projects.map((p) => (p.id === projectId ? { ...p, status: 'synced' } : p)) })),
    }),
    [state],
  );

  return <StoreContext.Provider value={value}>{children}</StoreContext.Provider>;
}

export function useStore() {
  const ctx = useContext(StoreContext);
  if (!ctx) throw new Error('useStore must be used inside <StoreProvider>');
  return ctx;
}

export { SONG_SECTIONS };
