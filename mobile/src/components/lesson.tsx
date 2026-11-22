import { Check, X } from 'lucide-react-native';
import type { ReactNode } from 'react';
import { Pressable, Text, View } from 'react-native';
import { useSafeAreaInsets } from 'react-native-safe-area-context';
import { colors, fonts } from '../theme';
import { Button, Row, T } from './ui';

export function OptionButton({ label, selected, onPress, state }: { label: string; selected: boolean; onPress: () => void; state?: 'correct' | 'wrong' }) {
  const border = state === 'correct' ? colors.green : state === 'wrong' ? colors.red : selected ? colors.coral : colors.raised;
  const bg = state === 'correct' ? colors.greenBg : state === 'wrong' ? colors.redBg : selected ? colors.coralBg : 'transparent';
  return (
    <Pressable
      accessibilityRole="radio"
      accessibilityState={{ checked: selected }}
      onPress={onPress}
      style={({ pressed }) => ({ height: 56, paddingHorizontal: 16, borderRadius: 14, borderWidth: 2, borderColor: border, backgroundColor: bg, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', opacity: pressed ? 0.85 : 1 })}
    >
      <Text style={{ fontFamily: selected ? fonts.semibold : fonts.medium, fontSize: 16, color: selected ? colors.text : '#D4D4D8' }}>{label}</Text>
      {selected ? <Check size={20} color={state === 'wrong' ? colors.red : state === 'correct' ? colors.green : colors.coral} strokeWidth={2.25} /> : null}
    </Pressable>
  );
}

// Duolingo-style result tray that slides over the bottom of a lesson screen.
export function ResultSheet({
  correct,
  title,
  body,
  actions,
  onContinue,
}: {
  correct: boolean;
  title: string;
  body?: string;
  actions?: ReactNode;
  onContinue: () => void;
}) {
  const insets = useSafeAreaInsets();
  const tone = correct
    ? { bg: colors.greenBg, fg: colors.green, text: '#D3F0E0', btn: 'primary' as const }
    : { bg: colors.redBg, fg: colors.red, text: '#F0D3D4', btn: 'danger' as const };
  const Icon = correct ? Check : X;
  return (
    <>
      <View pointerEvents="none" style={{ position: 'absolute', top: 0, left: 0, right: 0, bottom: 0, backgroundColor: 'rgba(8,8,10,0.35)' }} />
      <View
        accessibilityRole="alert"
        style={{ position: 'absolute', left: 0, right: 0, bottom: 0, backgroundColor: tone.bg, borderTopLeftRadius: 24, borderTopRightRadius: 24, paddingHorizontal: 20, paddingTop: 24, paddingBottom: Math.max(insets.bottom, 16) + 16, gap: 12 }}
      >
        <Row gap={10}>
          <View style={{ width: 32, height: 32, borderRadius: 16, backgroundColor: 'rgba(255,255,255,0.08)', alignItems: 'center', justifyContent: 'center' }}>
            <Icon size={18} color={tone.fg} strokeWidth={2.25} />
          </View>
          <T v="section" style={{ color: tone.fg, fontSize: 20 }}>{title}</T>
        </Row>
        {body ? <T style={{ color: tone.text, lineHeight: 23 }}>{body}</T> : null}
        {actions}
        <Button label={correct ? 'Continue' : 'Got it'} variant={tone.btn} onPress={onContinue} style={correct ? { backgroundColor: colors.green } : undefined} />
      </View>
    </>
  );
}
