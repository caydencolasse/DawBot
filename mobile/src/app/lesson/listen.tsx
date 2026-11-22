import { router } from 'expo-router';
import { MessageCircle, Play, Repeat } from 'lucide-react-native';
import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { ResultSheet } from '../../components/lesson';
import { Button, Chip, LessonHeader, Row, Screen, T, Waveform } from '../../components/ui';
import { LISTEN_QUESTION } from '../../data/lessons';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

function LoopCard({ letter, selected, onPress, pump, seed }: { letter: 'A' | 'B'; selected: boolean; onPress: () => void; pump?: boolean; seed: number }) {
  return (
    <Pressable
      accessibilityRole="radio"
      accessibilityState={{ checked: selected }}
      accessibilityLabel={`Loop ${letter}`}
      onPress={onPress}
      style={{ padding: 16, gap: 14, borderRadius: 16, borderWidth: 2, borderColor: selected ? colors.coral : colors.raised, backgroundColor: selected ? colors.coralBg : 'transparent' }}
    >
      <Row style={{ justifyContent: 'space-between' }}>
        <Row gap={10}>
          {/* TODO(audio): play the A/B loops with expo-audio */}
          <View style={{ width: 36, height: 36, borderRadius: 18, backgroundColor: selected ? colors.coral : colors.raised, alignItems: 'center', justifyContent: 'center' }}>
            <Play size={16} color={selected ? colors.onCoral : colors.text} fill={selected ? colors.onCoral : colors.text} />
          </View>
          <Text style={{ fontFamily: fonts.semibold, fontSize: 16, color: colors.text }}>Loop {letter}</Text>
        </Row>
        <T v="muted">0:08 · 124 BPM</T>
      </Row>
      <Waveform bars={36} seed={seed} height={44} barWidth={4} color={selected ? colors.coral : '#6E6E76'} pump={pump} />
    </Pressable>
  );
}

export default function ListenQuestion() {
  const { streak } = useStore();
  const [picked, setPicked] = useState<'A' | 'B' | null>(null);
  const [checked, setChecked] = useState(false);
  const q = LISTEN_QUESTION;
  const correct = picked === q.answer;

  return (
    <Screen footer={!checked ? <Button label="Check" disabled={!picked} onPress={() => setChecked(true)} /> : undefined}>
      <LessonHeader value={0.5} streak={streak} />
      <View style={{ paddingHorizontal: 20, paddingTop: 20, gap: 8 }}>
        <T v="eyebrow">Train your ears</T>
        <T v="heading">{q.prompt}</T>
        <T v="sub">{q.hint}</T>
      </View>
      <View accessibilityRole="radiogroup" style={{ paddingHorizontal: 20, paddingTop: 24, gap: 12 }}>
        <LoopCard letter="A" seed={5} selected={picked === 'A'} onPress={() => !checked && setPicked('A')} />
        <LoopCard letter="B" seed={9} pump selected={picked === 'B'} onPress={() => !checked && setPicked('B')} />
      </View>
      <Button label="Play both, one after the other" variant="ghost" icon={Repeat} compact style={{ marginTop: 8 }} />
      {checked ? (
        <ResultSheet
          correct={correct}
          title={correct ? 'You heard it' : 'Not quite'}
          body={correct ? 'That pumping pad is sidechain compression, triggered by the kick' : q.explanation}
          actions={
            !correct ? (
              <Row gap={8}>
                <Chip small label="Hear loop B again" icon={Play} />
                <Chip small label="Ask dawbot why" icon={MessageCircle} onPress={() => router.push('/ask' as never)} />
              </Row>
            ) : undefined
          }
          onContinue={() => router.replace('/lesson/order' as never)}
        />
      ) : null}
    </Screen>
  );
}
