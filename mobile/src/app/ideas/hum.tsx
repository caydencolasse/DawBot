import { router } from 'expo-router';
import { Check, ChevronRight, Download, Ellipsis, Play } from 'lucide-react-native';
import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { Button, Card, IconButton, NavHeader, Pill, Row, Screen, SectionHeader, T, Toggle, Waveform } from '../../components/ui';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

// [start in 16ths, length in 16ths, pitch row 0..10]
const NOTES: [number, number, number][] = [
  [0, 1.5, 5], [1.5, 0.5, 6], [2, 1, 8], [3, 1, 6], [4, 1.5, 5], [5.5, 0.5, 3], [6, 2, 5],
  [8, 1.5, 5], [9.5, 0.5, 6], [10, 1, 8], [11, 1, 10], [12, 2, 8], [14, 1, 6], [15, 1, 5],
];
const ROWS = 11;
const ROW_H = 16;
const QUANTIZE = ['Off', '1/8', '1/16', '1/32'];

function PianoRoll() {
  return (
    <View accessibilityLabel="Piano roll of the hummed melody, 14 notes" style={{ height: ROWS * ROW_H, borderRadius: 12, backgroundColor: '#121214', overflow: 'hidden' }}>
      {Array.from({ length: ROWS }, (_, r) => (
        <View key={r} style={{ position: 'absolute', left: 0, right: 0, top: r * ROW_H, height: ROW_H, backgroundColor: [1, 3, 6, 8, 10].includes(r) ? '#17171A' : '#121214' }} />
      ))}
      {[1, 2, 3].map((b) => (
        <View key={b} style={{ position: 'absolute', top: 0, bottom: 0, left: `${b * 25}%`, width: 1, backgroundColor: 'rgba(255,255,255,0.08)' }} />
      ))}
      {NOTES.map(([start, len, pitch], i) => (
        <View key={i} style={{ position: 'absolute', left: `${(start / 16) * 100}%`, width: `${(len / 16) * 100 - 0.6}%`, top: (10 - pitch) * ROW_H + 2, height: ROW_H - 4, borderRadius: 3, backgroundColor: colors.coral }} />
      ))}
      <View style={{ position: 'absolute', top: 0, bottom: 0, left: '37%', width: 2, backgroundColor: colors.text }} />
    </View>
  );
}

export default function HumResult() {
  const { addIdea } = useStore();
  const [quantize, setQuantize] = useState('1/16');
  const [snap, setSnap] = useState(true);

  const save = () => {
    addIdea({ kind: 'hum', title: 'Hummed hook', meta: 'Just now · 0:14 · MIDI ready, F minor', projectId: 'night-drive', section: 'Hook' });
    router.replace('/projects/night-drive' as never);
  };

  return (
    <Screen
      scroll
      footer={
        <Row gap={10}>
          {/* TODO(export): write a .mid file and open the share sheet */}
          <Button label=".mid" variant="secondary" icon={Download} style={{ width: 120 }} />
          <Button label="Save to Night Drive" onPress={save} style={{ flex: 1 }} />
        </Row>
      }
    >
      <NavHeader title="Hummed hook" right={<IconButton icon={Ellipsis} label="More options" />} />
      <Row style={{ paddingHorizontal: 20 }}>
        <Pressable accessibilityRole="button" accessibilityLabel="Play" style={{ width: 44, height: 44, borderRadius: 22, backgroundColor: colors.coral, alignItems: 'center', justifyContent: 'center' }}>
          <Play size={18} color={colors.onCoral} fill={colors.onCoral} />
        </Pressable>
        <View style={{ flex: 1 }}>
          <Waveform bars={40} seed={2} played={15} height={32} />
        </View>
        <T v="muted">0:14</T>
      </Row>

      <Row style={{ paddingHorizontal: 20, paddingTop: 20, justifyContent: 'space-between' }}>
        <T v="section">MIDI</T>
        <Pill label="14 notes found" tone="green" icon={Check} />
      </Row>
      <View style={{ paddingHorizontal: 20, paddingTop: 12 }}>
        <PianoRoll />
      </View>

      <Row gap={10} style={{ paddingHorizontal: 20, paddingTop: 16 }}>
        {[
          ['Key', 'F minor'],
          ['Tempo', '92 BPM'],
        ].map(([label, value]) => (
          <Card key={label} onPress={() => {}} style={{ flex: 1, paddingHorizontal: 14, paddingVertical: 12 }}>
            <T v="muted">{label}</T>
            <Row style={{ justifyContent: 'space-between', marginTop: 2 }}>
              <T v="bodyStrong" style={{ fontSize: 16 }}>{value}</T>
              <ChevronRight size={16} color={colors.text3} />
            </Row>
          </Card>
        ))}
      </Row>

      <SectionHeader title="Quantize" style={{ paddingHorizontal: 20, paddingTop: 16 }} />
      <View style={{ marginHorizontal: 20, marginTop: 8, flexDirection: 'row', gap: 4, padding: 4, borderRadius: 12, backgroundColor: colors.card }}>
        {QUANTIZE.map((v) => {
          const on = v === quantize;
          return (
            <Pressable key={v} accessibilityRole="button" accessibilityState={{ selected: on }} onPress={() => setQuantize(v)} style={{ flex: 1, height: 36, borderRadius: 9, alignItems: 'center', justifyContent: 'center', backgroundColor: on ? colors.raised : 'transparent' }}>
              <Text style={{ fontFamily: fonts.medium, fontSize: 14, color: on ? colors.text : colors.text2 }}>{v}</Text>
            </Pressable>
          );
        })}
      </View>
      <Row style={{ paddingHorizontal: 20, paddingTop: 12, height: 60, justifyContent: 'space-between' }}>
        <T>Snap to the key</T>
        <Toggle value={snap} onChange={setSnap} label="Snap to the key" />
      </Row>
    </Screen>
  );
}
