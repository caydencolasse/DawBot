import { router } from 'expo-router';
import { ChevronRight, Flame, Laptop } from 'lucide-react-native';
import { useEffect, useRef } from 'react';
import { Image, Pressable, Text, View } from 'react-native';
import { images } from '../../assets';
import { Button, Card, Row, Screen, T } from '../../components/ui';
import { CURRENT_LESSON } from '../../data/lessons';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

function Stat({ label, value, accent }: { label: string; value: string; accent?: boolean }) {
  return (
    <Card style={{ flex: 1, paddingVertical: 14, alignItems: 'center' }}>
      <T v="muted">{label}</T>
      <Text style={{ fontFamily: fonts.semibold, fontSize: 22, color: accent ? colors.coral : colors.text, marginTop: 4, fontVariant: ['tabular-nums'] }}>{value}</Text>
    </Card>
  );
}

export default function LessonComplete() {
  const { streak, completeLesson } = useStore();
  const recorded = useRef(false);
  useEffect(() => {
    if (!recorded.current) {
      recorded.current = true;
      completeLesson(CURRENT_LESSON.xp, CURRENT_LESSON.minutes);
    }
  }, [completeLesson]);

  return (
    <Screen scroll footer={<Button label="Continue" onPress={() => router.dismissTo('/learn' as never)} />}>
      <View style={{ alignItems: 'center', paddingTop: 24, paddingHorizontal: 20, gap: 16 }}>
        <View style={{ width: 220, height: 240, borderRadius: 24, overflow: 'hidden', backgroundColor: '#E2E2E2' }}>
          <Image source={images.robotWave} resizeMode="cover" style={{ width: '100%', height: '100%' }} accessibilityLabel="The dawbot robot celebrating" />
        </View>
        <T v="title" style={{ fontSize: 28 }}>Lesson complete</T>
        <T v="sub" style={{ textAlign: 'center' }}>You can now build every chord in E minor</T>
      </View>
      <Row gap={10} style={{ paddingHorizontal: 20, paddingTop: 24 }}>
        <Stat label="XP earned" value={`+${CURRENT_LESSON.xp}`} accent />
        <Stat label="Accuracy" value="88%" />
        <Stat label="Time" value="4:52" />
      </Row>
      <Pressable accessibilityRole="button" onPress={() => router.push('/streak' as never)} style={{ marginHorizontal: 20, marginTop: 12, padding: 14, borderRadius: 14, backgroundColor: colors.coralBg, flexDirection: 'row', alignItems: 'center', gap: 12 }}>
        <Flame size={22} color={colors.coral} fill={colors.coral} />
        <Text style={{ flex: 1, fontFamily: fonts.regular, fontSize: 15, color: colors.coralMuted }}>
          <Text style={{ fontFamily: fonts.semibold, color: colors.text }}>{streak}-day streak. </Text>Come back tomorrow to keep it
        </Text>
        <ChevronRight size={18} color={colors.coralText} />
      </Pressable>
      <Card style={{ marginHorizontal: 20, marginTop: 12, padding: 16, gap: 12 }}>
        <Row style={{ alignItems: 'flex-start' }}>
          <Laptop size={20} color={colors.coralText} strokeWidth={1.75} />
          <View style={{ flex: 1 }}>
            <T v="bodyStrong">Try it on a real track</T>
            <T v="small" style={{ marginTop: 2 }}>The plugin builds this progression with you in Ableton Live</T>
          </View>
        </Row>
        <Button label="Send to studio" variant="secondary" compact href="/projects/send?lesson=chords-e-minor" />
      </Card>
    </Screen>
  );
}
