import { Check, ChevronRight, Clock, Flame, Headphones, Laptop, Piano, Shield, Star, Trophy, type LucideIcon } from 'lucide-react-native';
import { Pressable, Text, View } from 'react-native';
import { router } from 'expo-router';
import { Card, NavHeader, Row, Screen, SectionHeader, T } from '../components/ui';
import { useStore } from '../store';
import { colors, fonts } from '../theme';

const WEEK = [
  { d: 'M', done: true }, { d: 'T', done: true }, { d: 'W', done: true }, { d: 'T', done: true },
  { d: 'F', done: true }, { d: 'S', done: true }, { d: 'S', done: false },
];

const BADGES: { name: string; icon: LucideIcon; done: boolean; progress?: string }[] = [
  { name: 'First beat', icon: Flame, done: true },
  { name: 'Chord builder', icon: Piano, done: true },
  { name: '7-day streak', icon: Star, done: true },
  { name: 'Ear trained', icon: Headphones, done: false, progress: '6 / 10' },
  { name: 'Finisher', icon: Laptop, done: false, progress: '1 / 3' },
  { name: 'Night owl', icon: Clock, done: false, progress: '2 / 5' },
];

export default function Streak() {
  const { streak } = useStore();
  return (
    <Screen scroll>
      <NavHeader title="Streak" />
      <Row gap={16} style={{ paddingHorizontal: 20 }}>
        <Flame size={56} color={colors.coral} fill={colors.coral} />
        <View>
          <Text style={{ fontFamily: fonts.semibold, fontSize: 40, lineHeight: 44, color: colors.text, fontVariant: ['tabular-nums'] }}>{streak}</Text>
          <T v="sub">day streak · your best is 21</T>
        </View>
      </Row>
      <Card style={{ marginHorizontal: 20, marginTop: 20, padding: 16, flexDirection: 'row', justifyContent: 'space-between' }}>
        {WEEK.map((w, i) => (
          <View key={i} style={{ alignItems: 'center', gap: 6 }}>
            <T v="muted">{w.d}</T>
            <View style={{ width: 36, height: 36, borderRadius: 18, alignItems: 'center', justifyContent: 'center', backgroundColor: w.done ? colors.coral : 'transparent', borderWidth: w.done ? 0 : 2, borderStyle: 'dashed', borderColor: '#4A4A52' }}>
              {w.done ? <Check size={16} color={colors.onCoral} strokeWidth={2.5} /> : null}
            </View>
          </View>
        ))}
      </Card>
      <Card style={{ marginHorizontal: 20, marginTop: 12, paddingHorizontal: 16, paddingVertical: 14, flexDirection: 'row', alignItems: 'center', gap: 12 }}>
        <Shield size={20} color={colors.coralText} strokeWidth={1.75} />
        <T style={{ flex: 1, fontSize: 14 }}>1 streak freeze ready</T>
        <T v="muted">Covers a missed day</T>
      </Card>

      <SectionHeader title="Achievements" right="3 of 24" style={{ paddingHorizontal: 20, paddingTop: 28 }} />
      <View style={{ paddingHorizontal: 20, paddingTop: 16, flexDirection: 'row', flexWrap: 'wrap', rowGap: 20 }}>
        {BADGES.map((b) => (
          <View key={b.name} style={{ width: '33.33%', alignItems: 'center', gap: 6 }}>
            <View style={{ width: 64, height: 64, borderRadius: 32, alignItems: 'center', justifyContent: 'center', borderWidth: 2, borderColor: b.done ? colors.coral : colors.raised, backgroundColor: b.done ? colors.coralBg : colors.card }}>
              <b.icon size={26} color={b.done ? colors.coral : colors.text3} strokeWidth={1.75} />
            </View>
            <Text style={{ fontFamily: fonts.medium, fontSize: 13, color: colors.text, textAlign: 'center' }}>{b.name}</Text>
            <T v="muted">{b.done ? 'Earned' : b.progress}</T>
          </View>
        ))}
      </View>

      <Pressable accessibilityRole="button" onPress={() => router.push('/league' as never)} style={{ marginHorizontal: 20, marginTop: 28 }}>
        <Card style={{ padding: 16, flexDirection: 'row', alignItems: 'center', gap: 12 }}>
          <Trophy size={22} color={colors.coralText} strokeWidth={1.75} />
          <View style={{ flex: 1 }}>
            <T v="bodyStrong">Bronze league</T>
            <T v="muted">You’re 4th this week</T>
          </View>
          <ChevronRight size={18} color={colors.text3} />
        </Card>
      </Pressable>
    </Screen>
  );
}
