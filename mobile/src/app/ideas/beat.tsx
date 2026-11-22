import { router } from 'expo-router';
import { ChevronRight, Laptop, Play, Plus } from 'lucide-react-native';
import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { Banner, Button, Chip, NavHeader, Row, Screen } from '../../components/ui';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

const START: Record<string, number[]> = {
  Kick: [0, 4, 8, 10, 12],
  Snare: [4, 12],
  Clap: [4, 12, 15],
  Hat: [2, 6, 10, 14],
  'Hat 2': [0, 1, 3, 5, 7, 9, 11, 13, 15],
  Perc: [7, 11],
};

export default function BeatSketch() {
  const { addIdea } = useStore();
  const [grid, setGrid] = useState(() => Object.fromEntries(Object.entries(START).map(([k, v]) => [k, new Set(v)])) as Record<string, Set<number>>);
  const [pattern, setPattern] = useState<'A' | 'B'>('A');

  const toggle = (row: string, step: number) =>
    setGrid((g) => {
      const next = new Set(g[row]);
      if (next.has(step)) next.delete(step);
      else next.add(step);
      return { ...g, [row]: next };
    });

  const save = () => {
    addIdea({ kind: 'beat', title: 'Beat sketch', meta: 'Just now · 124 BPM · 2 bars', projectId: 'summer-flip', section: 'Intro' });
    router.replace('/projects/summer-flip' as never);
  };

  return (
    <Screen
      scroll
      footer={
        <Row gap={10}>
          {/* TODO(audio): sequence the pattern with drum samples */}
          <Pressable accessibilityRole="button" accessibilityLabel="Play pattern" style={{ width: 52, height: 52, borderRadius: 14, backgroundColor: colors.raised, alignItems: 'center', justifyContent: 'center' }}>
            <Play size={20} color={colors.text} fill={colors.text} />
          </Pressable>
          <Button label="Save idea" onPress={save} style={{ flex: 1 }} />
        </Row>
      }
    >
      <NavHeader title="Beat sketch" close />
      <Row gap={8} style={{ paddingHorizontal: 20 }}>
        <Chip label="124 BPM" on />
        <Chip label="Swing 12%" />
        <Chip label="House kit" icon={ChevronRight} />
      </Row>

      <Row gap={3} style={{ paddingHorizontal: 20, paddingTop: 24 }}>
        <View style={{ width: 48 }} />
        {[1, 2, 3, 4].map((b) => (
          <Text key={b} style={{ flex: 1, textAlign: 'center', fontFamily: fonts.regular, fontSize: 11, color: colors.text3, marginLeft: b > 1 ? 4 : 0 }}>{b}</Text>
        ))}
      </Row>
      <View style={{ paddingHorizontal: 20, paddingTop: 8, gap: 8 }}>
        {Object.keys(START).map((row) => (
          <Row key={row} gap={3}>
            <Text style={{ width: 48, fontFamily: fonts.regular, fontSize: 13, color: colors.text2 }}>{row}</Text>
            {Array.from({ length: 16 }, (_, i) => {
              const on = grid[row].has(i);
              return (
                <Pressable
                  key={i}
                  accessibilityRole="checkbox"
                  accessibilityState={{ checked: on }}
                  accessibilityLabel={`${row}, step ${i + 1}`}
                  onPress={() => toggle(row, i)}
                  style={{ flex: 1, height: 34, borderRadius: 5, marginLeft: i > 0 && i % 4 === 0 ? 4 : 0, backgroundColor: on ? colors.coral : Math.floor(i / 4) % 2 === 0 ? colors.raised : '#26262B' }}
                />
              );
            })}
          </Row>
        ))}
      </View>

      <Row gap={8} style={{ paddingHorizontal: 20, paddingTop: 20 }}>
        <Chip label="Pattern A" on={pattern === 'A'} onPress={() => setPattern('A')} />
        <Chip label="Pattern B" on={pattern === 'B'} onPress={() => setPattern('B')} />
        <Chip label="New" icon={Plus} />
      </Row>
      <Banner icon={Laptop} tone="neutral" body="Saved as a MIDI drum clip that maps to Drum Rack in Ableton Live" style={{ marginHorizontal: 20, marginTop: 20 }} />
    </Screen>
  );
}
