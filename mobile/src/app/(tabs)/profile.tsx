import { router } from 'expo-router';
import { BookOpen, ChevronRight, Flame, Mic, SlidersHorizontal, Star, Trophy, Zap, type LucideIcon } from 'lucide-react-native';
import { Pressable, Text, View } from 'react-native';
import { Card, DawIcon, IconButton, Pill, Row, Screen, T } from '../../components/ui';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

function Stat({ icon: Icon, value, label, coral }: { icon: LucideIcon; value: string; label: string; coral?: boolean }) {
  return (
    <Card style={{ width: '48.5%', paddingHorizontal: 16, paddingVertical: 14, gap: 6 }}>
      <Icon size={20} color={coral ? colors.coral : colors.coralText} fill={coral ? colors.coral : 'none'} strokeWidth={1.75} />
      <Text style={{ fontFamily: fonts.semibold, fontSize: 22, color: colors.text, fontVariant: ['tabular-nums'] }}>{value}</Text>
      <T v="muted">{label}</T>
    </Card>
  );
}

function LinkRow({ icon: Icon, title, value, href, last }: { icon: LucideIcon; title: string; value?: string; href: string; last?: boolean }) {
  return (
    <Pressable accessibilityRole="button" onPress={() => router.push(href as never)} style={{ height: 52, paddingHorizontal: 16, flexDirection: 'row', alignItems: 'center', gap: 12, borderBottomWidth: last ? 0 : 1, borderBottomColor: colors.line }}>
      <Icon size={20} color={colors.text2} strokeWidth={1.75} />
      <T style={{ flex: 1 }}>{title}</T>
      {value ? <T v="small">{value}</T> : null}
      <ChevronRight size={16} color={colors.text3} />
    </Pressable>
  );
}

export default function Profile() {
  const { streak, xp, ideas } = useStore();
  return (
    <Screen scroll>
      <Row gap={14} style={{ paddingHorizontal: 20, paddingTop: 8 }}>
        <View style={{ width: 56, height: 56, borderRadius: 28, backgroundColor: colors.coral, alignItems: 'center', justifyContent: 'center' }}>
          <Text style={{ fontFamily: fonts.semibold, fontSize: 20, color: colors.onCoral }}>CC</Text>
        </View>
        <View style={{ flex: 1 }}>
          <T v="section" style={{ fontSize: 22, lineHeight: 28 }}>Cayden</T>
          <T v="small" style={{ color: colors.text3 }}>Making tracks · since November 2026</T>
        </View>
        <IconButton icon={SlidersHorizontal} label="Settings" tone="card" size={20} href="/settings" />
      </Row>

      <View style={{ paddingHorizontal: 20, paddingTop: 20, flexDirection: 'row', flexWrap: 'wrap', gap: 10 }}>
        <Stat icon={Flame} value={String(streak)} label="Day streak" coral />
        <Stat icon={Zap} value={(2140 + xp).toLocaleString('en-US')} label="Total XP" />
        <Stat icon={BookOpen} value="42" label="Lessons" />
        <Stat icon={Mic} value={String(ideas.length + 14)} label="Ideas" />
      </View>

      <Card style={{ marginHorizontal: 20, marginTop: 12, padding: 16, gap: 12 }}>
        <Row style={{ justifyContent: 'space-between' }}>
          <T v="bodyStrong">Your studio</T>
          <Pill label="Linked" tone="green" />
        </Row>
        <Row>
          <DawIcon id="ableton" size={36} />
          <View style={{ flex: 1 }}>
            <T>MacBook Pro · Ableton Live 12</T>
            <T v="muted">dawbot plugin · VST3 and AU</T>
          </View>
        </Row>
        <Row style={{ justifyContent: 'space-between', paddingTop: 12, borderTopWidth: 1, borderTopColor: colors.line }}>
          <T v="small">Plugin plan</T>
          <T style={{ fontSize: 14 }}>Free trial · 5 days left</T>
        </Row>
      </Card>

      <Card style={{ marginHorizontal: 20, marginTop: 12, overflow: 'hidden' }}>
        <LinkRow icon={Trophy} title="Achievements" value="3 of 24" href="/streak" />
        <LinkRow icon={Star} title="League" value="Bronze · 4th" href="/league" last />
      </Card>
    </Screen>
  );
}
