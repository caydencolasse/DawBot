import { router } from 'expo-router';
import { ChevronDown, ChevronUp, Laptop } from 'lucide-react-native';
import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { ResultSheet } from '../../components/lesson';
import { Banner, Button, LessonHeader, Screen, T } from '../../components/ui';
import { ORDER_QUESTION } from '../../data/lessons';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

// Reorder with the up/down buttons (works with screen readers too).
// A drag handle with react-native-gesture-handler can be layered on later.
export default function OrderQuestion() {
  const { streak } = useStore();
  const q = ORDER_QUESTION;
  const [order, setOrder] = useState<number[]>(q.start);
  const [checked, setChecked] = useState(false);
  const correct = order.every((v, i) => v === i);

  const move = (from: number, to: number) => {
    if (to < 0 || to >= order.length) return;
    const next = [...order];
    [next[from], next[to]] = [next[to], next[from]];
    setOrder(next);
  };

  return (
    <Screen footer={!checked ? <Button label="Check" onPress={() => setChecked(true)} /> : undefined}>
      <LessonHeader value={0.75} streak={streak} />
      <View style={{ paddingHorizontal: 20, paddingTop: 20, gap: 8 }}>
        <T v="eyebrow">Your DAW · {q.daw}</T>
        <T v="heading">{q.prompt}</T>
        <T v="sub">Move each step into place</T>
      </View>
      <View style={{ paddingHorizontal: 20, paddingTop: 24, gap: 10 }}>
        {order.map((stepIndex, pos) => (
          <View key={stepIndex} style={{ minHeight: 60, flexDirection: 'row', alignItems: 'center', gap: 12, paddingLeft: 16, paddingRight: 4, borderRadius: 14, backgroundColor: colors.card, borderWidth: 1, borderColor: checked ? (stepIndex === pos ? colors.green : colors.red) : colors.line }}>
            <Text style={{ fontFamily: fonts.semibold, fontSize: 14, color: colors.text3, width: 16 }}>{pos + 1}</Text>
            <Text style={{ flex: 1, fontFamily: fonts.regular, fontSize: 15, color: colors.text, paddingVertical: 10 }}>{q.correct[stepIndex]}</Text>
            <Pressable accessibilityRole="button" accessibilityLabel="Move up" disabled={checked || pos === 0} onPress={() => move(pos, pos - 1)} style={{ width: 40, height: 44, alignItems: 'center', justifyContent: 'center', opacity: pos === 0 ? 0.3 : 1 }}>
              <ChevronUp size={20} color={colors.text2} strokeWidth={1.75} />
            </Pressable>
            <Pressable accessibilityRole="button" accessibilityLabel="Move down" disabled={checked || pos === order.length - 1} onPress={() => move(pos, pos + 1)} style={{ width: 40, height: 44, alignItems: 'center', justifyContent: 'center', opacity: pos === order.length - 1 ? 0.3 : 1 }}>
              <ChevronDown size={20} color={colors.text2} strokeWidth={1.75} />
            </Pressable>
          </View>
        ))}
      </View>
      <Banner icon={Laptop} tone="neutral" body="After this lesson, the plugin can walk you through it on your own track" style={{ marginHorizontal: 20, marginTop: 20 }} />
      {checked ? (
        <ResultSheet
          correct={correct}
          title={correct ? 'Perfect order' : 'Close'}
          body={correct ? undefined : 'Add the effect first, then open its automation lane before you draw'}
          onContinue={() => (correct ? router.replace('/lesson/complete' as never) : setChecked(false))}
        />
      ) : null}
    </Screen>
  );
}
