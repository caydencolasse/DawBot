import { LayoutGrid, Mic, PenLine, Piano, type LucideIcon } from 'lucide-react-native';
import type { IdeaKind } from '../store';

export const IDEA_ICONS: Record<IdeaKind, LucideIcon> = {
  hum: Mic,
  chords: Piano,
  beat: LayoutGrid,
  note: PenLine,
};

export const IDEA_ROUTES: Record<IdeaKind, string> = {
  hum: '/ideas/hum',
  chords: '/ideas/chords',
  beat: '/ideas/beat',
  note: '/ideas/hum',
};
