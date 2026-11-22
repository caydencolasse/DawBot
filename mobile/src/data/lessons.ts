export type PathNode = {
  id: string;
  title: string;
  state: 'done' | 'current' | 'locked' | 'review';
  offset: number; // horizontal zig-zag offset on the path
};

export const UNIT = { number: 3, title: 'Minor keys & moods' };

export const PATH: PathNode[] = [
  { id: 'rhythm', title: 'Rhythm basics', state: 'done', offset: -70 },
  { id: 'scales', title: 'Scales on the keys', state: 'done', offset: 40 },
  { id: 'chords-e-minor', title: 'Chords in E minor', state: 'current', offset: -16 },
  { id: 'bass', title: 'Bass that follows chords', state: 'locked', offset: 64 },
  { id: 'review', title: 'Unit 3 review', state: 'review', offset: -48 },
];

export const CURRENT_LESSON = {
  id: 'chords-e-minor',
  title: 'Chords in E minor',
  meta: 'Unit 3 · Lesson 3 of 5',
  minutes: 5,
  xp: 15,
  exercises: 8,
  outcomes: ['Find E minor on the keys', 'Build the i, iv, VI and VII chords', 'Hear why minor sounds moody'],
};

export const CHORD_QUESTION = {
  prompt: 'Which notes make an E minor chord?',
  options: ['E · G · B', 'E · G♯ · B', 'E · A · C'],
  answer: 0,
  notes: [['E', 'G', 'B'], ['E', 'G', 'B'], ['E', 'A', 'C']] as string[][],
};

export const LISTEN_QUESTION = {
  prompt: 'Which loop uses sidechain compression?',
  hint: 'Listen for the pad ducking each time the kick hits',
  answer: 'B' as 'A' | 'B',
  explanation:
    'Loop B is the sidechained one. Its pad drops in level every time the kick hits, which creates that pumping feel. Loop A stays at a steady level',
};

export const ORDER_QUESTION = {
  prompt: 'Put the steps in order to automate a filter',
  daw: 'Ableton Live 12',
  correct: [
    'Add Auto Filter to the track',
    'Press A to show automation lanes',
    'Draw the cutoff curve with the pencil',
    'Play it back and adjust the shape',
  ],
  start: [1, 0, 2, 3], // shuffled indices into `correct`
};
