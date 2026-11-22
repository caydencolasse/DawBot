import { router } from 'expo-router';
import { ChevronRight, MessageCircle, Play, Sparkles, X } from 'lucide-react-native';
import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { Banner, Button, Chip, NavHeader, Row, Screen, SectionHeader } from '../../components/ui';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

// Diatonic chords of F minor.
const PADS = [
  { roman: 'i', chord: 'Fm' }, { roman: 'ii°', chord: 'Gdim' }, { roman: 'III', chord: 'A♭' }, { roman: 'iv', chord: 'B♭m' },
  { roman: 'v', chord: 'Cm' }, { roman: 'VI', chord: 'D♭' }, { roman: 'VII', chord: 'E♭' },
];

export default function ChordSketch() {
  const { addIdea } = useStore();
  const [slots, setSlots] = useState(['i', 'VI', 'III', 'VII']);
  const [active, setActive] = useState(0);
  const chordFor = (r: string) => PADS.find((p) => p.roman === r)?.chord ?? r;

  const addPad = (roman: string) => setSlots((s) => (s.length >= 8 ? s : [...s, roman]));
  const suggest = () => addPad('v');
  const remove = (i: number) => setSlots((s) => s.filter((_, j) => j !== i));

  const save = () => {
    addIdea({ kind: 'chords', title: 'Chord loop', meta: `Just now · ${slots.map(chordFor).join(' ')} · 92 BPM`, projectId: 'night-drive', section: 'Verse' });
    router.replace('/projects/night-drive' as never);
  };

  return (
    <Screen
      scroll
      footer={
        <Row gap={10}>
          {/* TODO(audio): play the progression with a piano sample */}
          <Pressable accessibilityRole="button" accessibilityLabel="Play progression" onPress={() => setActive((a) => (a + 1) % slots.length)} style={{ width: 52, height: 52, borderRadius: 14, backgroundColor: colors.raised, alignItems: 'center', justifyContent: 'center' }}>
            <Play size={20} color={colors.text} fill={colors.text} />
          </Pressable>
          <Button label="Save idea" onPress={save} style={{ flex: 1 }} />
        </Row>
      }
    >
      <NavHeader title="Chord sketch" close />
      <Row gap={8} style={{ paddingHorizontal: 20 }}>
        <Chip label="F minor" on icon={ChevronRight} />
        <Chip label="92 BPM" />
        <Chip label="Piano" />
      </Row>

      <SectionHeader title="Progression" right={`${slots.length} bars`} style={{ paddingHorizontal: 20, paddingTop: 24 }} />
      <View style={{ paddingHorizontal: 20, paddingTop: 12, flexDirection: 'row', flexWrap: 'wrap', gap: 8 }}>
        {slots.map((r, i) => {
          const on = i === active;
          return (
            <Pressable key={`${r}-${i}`} accessibilityRole="button" accessibilityLabel={`Bar ${i + 1}, ${chordFor(r)}. Long press to remove`} onPress={() => setActive(i)} onLongPress={() => remove(i)} style={{ width: '23.3%', height: 76, borderRadius: 14, alignItems: 'center', justifyContent: 'center', gap: 2, backgroundColor: on ? colors.coral : colors.card, borderWidth: on ? 0 : 1, borderColor: colors.line }}>
              <Text style={{ fontFamily: fonts.semibold, fontSize: 20, color: on ? colors.onCoral : colors.text }}>{chordFor(r)}</Text>
              <Text style={{ fontFamily: fonts.medium, fontSize: 12, color: on ? colors.onCoral : colors.text3 }}>{r}</Text>
            </Pressable>
          );
        })}
        {slots.length > 0 ? (
          <Pressable accessibilityRole="button" accessibilityLabel="Remove last chord" onPress={() => remove(slots.length - 1)} style={{ width: '23.3%', height: 76, borderRadius: 14, alignItems: 'center', justifyContent: 'center', borderWidth: 1, borderStyle: 'dashed', borderColor: '#4A4A52' }}>
            <X size={20} color={colors.text3} />
          </Pressable>
        ) : null}
      </View>

      <SectionHeader title="Chords in F minor" right="Tap to add" style={{ paddingHorizontal: 20, paddingTop: 24 }} />
      <View style={{ paddingHorizontal: 20, paddingTop: 12, flexDirection: 'row', flexWrap: 'wrap', gap: 8 }}>
        {PADS.map((p) => (
          <Pressable key={p.roman} accessibilityRole="button" accessibilityLabel={`Add ${p.chord}`} onPress={() => addPad(p.roman)} style={({ pressed }) => ({ width: '23.3%', height: 64, borderRadius: 14, alignItems: 'center', justifyContent: 'center', gap: 2, backgroundColor: pressed ? colors.coralBg : colors.raised, borderWidth: 1, borderColor: colors.line })}>
            <Text style={{ fontFamily: fonts.semibold, fontSize: 16, color: colors.text }}>{p.chord}</Text>
            <Text style={{ fontFamily: fonts.medium, fontSize: 12, color: colors.text3 }}>{p.roman}</Text>
          </Pressable>
        ))}
        <Pressable accessibilityRole="button" accessibilityLabel="Suggest the next chord" onPress={suggest} style={{ width: '23.3%', height: 64, borderRadius: 14, alignItems: 'center', justifyContent: 'center', gap: 4, borderWidth: 1, borderStyle: 'dashed', borderColor: '#4A4A52' }}>
          <Sparkles size={18} color={colors.coralText} strokeWidth={1.75} />
          <Text style={{ fontFamily: fonts.semibold, fontSize: 13, color: colors.coralText }}>Suggest</Text>
        </Pressable>
      </View>

      <Banner icon={MessageCircle} body="i, VI, III, VII is a classic moody loop. Try the v chord (Cm) in bar 4 for more tension" style={{ marginHorizontal: 20, marginTop: 20 }} />
    </Screen>
  );
}
