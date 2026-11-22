import { router } from 'expo-router';
import { Book, Check, Flame, Lock, Play, Trophy, Zap } from 'lucide-react-native';
import { Pressable, Text, View } from 'react-native';
import { Card, DawPill, IconButton, ProgressBar, Row, Screen, T } from '../../components/ui';
import { PATH, UNIT, type PathNode } from '../../data/lessons';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

function Node({ node }: { node: PathNode }) {
  const big = node.state === 'current';
  const size = big ? 84 : 64;
  const circle = {
    done: { bg: colors.raised, border: colors.coral, fg: colors.coral },
    current: { bg: colors.coral, border: colors.coral, fg: colors.onCoral },
    locked: { bg: colors.card, border: colors.card, fg: colors.text3 },
    review: { bg: colors.card, border: '#4A4A52', fg: colors.text3 },
  }[node.state];
  const Icon = { done: Check, current: Play, locked: Lock, review: Trophy }[node.state];
  return (
    <Pressable
      accessibilityRole="button"
      accessibilityLabel={`${node.title}, ${node.state === 'done' ? 'completed' : node.state === 'current' ? 'start lesson' : node.state}`}
      onPress={() => node.state !== 'locked' && router.push('/lesson/start' as never)}
      style={{ alignItems: 'center', gap: 8, transform: [{ translateX: node.offset }] }}
    >
      <View
        style={{
          width: size,
          height: size,
          borderRadius: size / 2,
          backgroundColor: circle.bg,
          borderWidth: node.state === 'done' ? 3 : node.state === 'review' ? 1 : 0,
          borderStyle: node.state === 'review' ? 'dashed' : 'solid',
          borderColor: circle.border,
          alignItems: 'center',
          justifyContent: 'center',
          ...(big ? { boxShadow: '0 0 0 8px rgba(244,97,63,0.2)' } : null),
        }}
      >
        <Icon size={big ? 32 : 24} color={circle.fg} fill={node.state === 'current' ? circle.fg : 'none'} strokeWidth={node.state === 'done' ? 2.5 : 1.75} />
      </View>
      <Text style={{ fontFamily: big ? fonts.semibold : fonts.regular, fontSize: big ? 14 : 13, color: node.state === 'done' ? colors.text2 : big ? colors.text : colors.text3 }}>
        {node.title}
      </Text>
    </Pressable>
  );
}

export default function Learn() {
  const { streak, xp, minutesToday, dailyMinutes } = useStore();
  return (
    <Screen scroll>
      <Row style={{ paddingHorizontal: 20, paddingTop: 8, justifyContent: 'space-between' }}>
        <DawPill />
        <Row gap={16}>
          <Pressable accessibilityRole="button" accessibilityLabel={`${streak} day streak`} onPress={() => router.push('/streak' as never)}>
            <Row gap={4}>
              <Flame size={20} color={colors.coral} fill={colors.coral} />
              <T v="bodyStrong">{streak}</T>
            </Row>
          </Pressable>
          <Row gap={4} style={{}}>
            <Zap size={18} color={colors.text2} strokeWidth={1.75} />
            <T v="bodyStrong" style={{ color: colors.text2 }}>{xp}</T>
          </Row>
        </Row>
      </Row>

      <Card style={{ marginHorizontal: 20, marginTop: 16, paddingHorizontal: 16, paddingVertical: 14, gap: 10 }}>
        <Row style={{ justifyContent: 'space-between' }}>
          <T v="bodyStrong" style={{ fontSize: 14 }}>Daily goal</T>
          <T v="small" style={{ fontVariant: ['tabular-nums'] }}>
            {Math.min(minutesToday, dailyMinutes)} of {dailyMinutes} min
          </T>
        </Row>
        <ProgressBar value={minutesToday / dailyMinutes} />
      </Card>

      <View style={{ marginHorizontal: 20, marginTop: 12, padding: 16, borderRadius: 16, backgroundColor: colors.coral, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' }}>
        <View>
          <Text style={{ fontFamily: fonts.semibold, fontSize: 13, color: colors.onCoral }}>Unit {UNIT.number}</Text>
          <Text style={{ fontFamily: fonts.semibold, fontSize: 19, color: colors.onCoral, marginTop: 2 }}>{UNIT.title}</Text>
        </View>
        <View style={{ backgroundColor: 'rgba(20,20,22,0.12)', borderRadius: 12 }}>
          <IconButton icon={Book} label="Unit guidebook" />
        </View>
      </View>

      <View style={{ paddingTop: 24, alignItems: 'center', gap: 18 }}>
        {PATH.map((n) => (
          <Node key={n.id} node={n} />
        ))}
      </View>
    </Screen>
  );
}
