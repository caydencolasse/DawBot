import { router } from 'expo-router';
import { Bell, ChevronRight, CircleQuestionMark, CreditCard, Laptop, LogOut, Music, RefreshCw, Shield, Target, Volume2, type LucideIcon } from 'lucide-react-native';
import { useState, type ReactNode } from 'react';
import { Pressable, View } from 'react-native';
import { Button, Card, NavHeader, Screen, T, Toggle } from '../components/ui';
import { useStore } from '../store';
import { colors } from '../theme';

function Group({ title, children }: { title: string; children: ReactNode }) {
  return (
    <View style={{ gap: 8, paddingTop: 16 }}>
      <T v="muted" style={{ fontFamily: 'Inter_600SemiBold', paddingHorizontal: 4 }}>{title}</T>
      <Card style={{ overflow: 'hidden' }}>{children}</Card>
    </View>
  );
}

function NavRow({ icon: Icon, title, value, last }: { icon: LucideIcon; title: string; value?: string; last?: boolean }) {
  return (
    <Pressable accessibilityRole="button" style={{ height: 52, paddingHorizontal: 16, flexDirection: 'row', alignItems: 'center', gap: 12, borderBottomWidth: last ? 0 : 1, borderBottomColor: colors.line }}>
      <Icon size={20} color={colors.text2} strokeWidth={1.75} />
      <T style={{ flex: 1 }}>{title}</T>
      {value ? <T v="small">{value}</T> : null}
      <ChevronRight size={16} color={colors.text3} />
    </Pressable>
  );
}

function SwitchRow({ icon: Icon, title, sub, value, onChange, last }: { icon: LucideIcon; title: string; sub?: string; value: boolean; onChange: (v: boolean) => void; last?: boolean }) {
  return (
    <View style={{ minHeight: 60, paddingHorizontal: 16, paddingVertical: 8, flexDirection: 'row', alignItems: 'center', gap: 12, borderBottomWidth: last ? 0 : 1, borderBottomColor: colors.line }}>
      <Icon size={20} color={colors.text2} strokeWidth={1.75} />
      <View style={{ flex: 1 }}>
        <T>{title}</T>
        {sub ? <T v="muted">{sub}</T> : null}
      </View>
      <Toggle value={value} onChange={onChange} label={title} />
    </View>
  );
}

export default function Settings() {
  const { dailyMinutes } = useStore();
  const [reminder, setReminder] = useState(true);
  const [sounds, setSounds] = useState(true);
  const [autoSend, setAutoSend] = useState(true);

  return (
    <Screen scroll>
      <NavHeader title="Settings" />
      <View style={{ paddingHorizontal: 20 }}>
        <Group title="Learning">
          <NavRow icon={Target} title="Daily goal" value={`${dailyMinutes} min`} />
          <SwitchRow icon={Bell} title="Practice reminder" sub="7:30 PM every day" value={reminder} onChange={setReminder} />
          <SwitchRow icon={Volume2} title="Lesson sounds" value={sounds} onChange={setSounds} last />
        </Group>
        <Group title="Studio">
          <NavRow icon={Music} title="Default DAW" value="Ableton Live 12" />
          <NavRow icon={Laptop} title="Linked computer" value="MacBook Pro" />
          <SwitchRow icon={RefreshCw} title="Auto-send new ideas" sub="Ideas in a project go to the plugin" value={autoSend} onChange={setAutoSend} last />
        </Group>
        <Group title="Account">
          <NavRow icon={CreditCard} title="Subscription" value="Free trial" />
          <NavRow icon={Shield} title="Privacy and data" />
          <NavRow icon={CircleQuestionMark} title="Help and support" last />
        </Group>
        <Button label="Sign out" variant="ghost" icon={LogOut} onPress={() => router.replace('/onboarding' as never)} style={{ marginTop: 8 }} />
      </View>
    </Screen>
  );
}
