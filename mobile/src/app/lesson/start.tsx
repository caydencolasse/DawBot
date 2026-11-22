import { router } from 'expo-router';
import { Check, Clock, Laptop, Target, Zap } from 'lucide-react-native';
import { Pressable, View } from 'react-native';
import { useSafeAreaInsets } from 'react-native-safe-area-context';
import { Button, Row, T } from '../../components/ui';
import { CURRENT_LESSON } from '../../data/lessons';
import { colors } from '../../theme';

// Presented as a transparent modal over the Learn path.
export default function LessonStart() {
  const insets = useSafeAreaInsets();
  const l = CURRENT_LESSON;
  return (
    <View style={{ flex: 1, justifyContent: 'flex-end' }}>
      <Pressable accessibilityRole="button" accessibilityLabel="Close" onPress={() => router.back()} style={{ position: 'absolute', top: 0, left: 0, right: 0, bottom: 0, backgroundColor: colors.dim }} />
      <View accessibilityViewIsModal style={{ backgroundColor: colors.sheet, borderTopLeftRadius: 24, borderTopRightRadius: 24, borderTopWidth: 1, borderColor: colors.line, paddingHorizontal: 20, paddingTop: 12, paddingBottom: Math.max(insets.bottom, 16) + 16 }}>
        <View style={{ width: 36, height: 5, borderRadius: 3, backgroundColor: 'rgba(255,255,255,0.2)', alignSelf: 'center', marginBottom: 16 }} />
        <T v="eyebrow">{l.meta}</T>
        <T v="heading" style={{ marginTop: 6 }}>{l.title}</T>
        <Row gap={16} style={{ marginTop: 12 }}>
          <Row gap={6}><Clock size={16} color={colors.text2} strokeWidth={1.75} /><T v="small">{l.minutes} min</T></Row>
          <Row gap={6}><Zap size={16} color={colors.text2} strokeWidth={1.75} /><T v="small">{l.xp} XP</T></Row>
          <Row gap={6}><Target size={16} color={colors.text2} strokeWidth={1.75} /><T v="small">{l.exercises} exercises</T></Row>
        </Row>
        <View style={{ marginTop: 20, gap: 10 }}>
          {l.outcomes.map((o) => (
            <Row key={o}>
              <Check size={18} color={colors.coralText} strokeWidth={2} />
              <T>{o}</T>
            </Row>
          ))}
        </View>
        <View style={{ marginTop: 24, gap: 8 }}>
          <Button label="Start lesson" onPress={() => router.replace('/lesson/chord' as never)} />
          <Button label="Practice it in my DAW instead" variant="secondary" icon={Laptop} onPress={() => router.replace('/projects/send?lesson=chords-e-minor' as never)} />
        </View>
      </View>
    </View>
  );
}
