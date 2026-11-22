import { router } from 'expo-router';
import { Volume2 } from 'lucide-react-native';
import { useState } from 'react';
import { View } from 'react-native';
import { OptionButton, ResultSheet } from '../../components/lesson';
import { Button, LessonHeader, Piano, Row, Screen, T } from '../../components/ui';
import { CHORD_QUESTION } from '../../data/lessons';
import { useStore } from '../../store';
import { colors } from '../../theme';

export default function ChordQuestion() {
  const { streak } = useStore();
  const [picked, setPicked] = useState<number | null>(null);
  const [checked, setChecked] = useState(false);
  const q = CHORD_QUESTION;
  const correct = picked === q.answer;

  return (
    <Screen footer={!checked ? <Button label="Check" disabled={picked === null} onPress={() => setChecked(true)} /> : undefined}>
      <LessonHeader value={0.38} streak={streak} />
      <View style={{ paddingHorizontal: 20, paddingTop: 20, gap: 8 }}>
        <T v="eyebrow">Build the chord</T>
        <T v="heading">{q.prompt}</T>
      </View>
      <View style={{ paddingHorizontal: 20, paddingTop: 24 }}>
        <Piano highlight={picked === null ? [] : q.notes[picked]} />
      </View>
      <Row gap={8} style={{ paddingHorizontal: 20, paddingTop: 12 }}>
        <Volume2 size={18} color={colors.text2} strokeWidth={1.75} />
        <T v="small">Tap the keys to hear them</T>
      </Row>
      <View accessibilityRole="radiogroup" style={{ paddingHorizontal: 20, paddingTop: 20, gap: 10 }}>
        {q.options.map((o, i) => (
          <OptionButton
            key={o}
            label={o}
            selected={picked === i}
            state={checked && picked === i ? (correct ? 'correct' : 'wrong') : undefined}
            onPress={() => !checked && setPicked(i)}
          />
        ))}
      </View>
      {checked ? (
        <ResultSheet
          correct={correct}
          title={correct ? 'Nice, that is E minor' : 'Not quite'}
          body={correct ? 'E is the root, G is the minor third and B is the fifth' : 'A minor chord stacks a minor third (E to G) and a perfect fifth (E to B)'}
          onContinue={() => (correct ? router.replace('/lesson/listen' as never) : (setChecked(false), setPicked(null)))}
        />
      ) : null}
    </Screen>
  );
}
