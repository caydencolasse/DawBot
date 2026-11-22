import { View } from 'react-native';
import { Button, Radio, Screen, SelectCard, StepHeader, T, Title } from '../../components/ui';
import { useStore, type Level } from '../../store';

const LEVELS: { id: Level; title: string; body: string }[] = [
  { id: 'new', title: 'Brand new', body: 'I have never opened a DAW' },
  { id: 'starting', title: 'Getting started', body: 'I have made a few loops' },
  { id: 'tracks', title: 'Making tracks', body: 'I finish songs but want them to sound better' },
  { id: 'releasing', title: 'Releasing music', body: 'I want faster workflows and pro techniques' },
];

export default function PickLevel() {
  const { level, setLevel } = useStore();
  return (
    <Screen scroll footer={<Button label="Continue" href="/onboarding/goals" />}>
      <StepHeader value={0.5} />
      <Title title="How much have you made so far?" sub="We'll start you at the right unit and skip what you know" style={{ paddingHorizontal: 20, paddingTop: 8 }} />
      <View accessibilityRole="radiogroup" style={{ padding: 20, paddingTop: 24, gap: 12 }}>
        {LEVELS.map((l) => (
          <SelectCard key={l.id} selected={level === l.id} onPress={() => setLevel(l.id)} style={{ flexDirection: 'row', alignItems: 'center', gap: 14, padding: 16 }}>
            <Radio on={level === l.id} />
            <View style={{ flex: 1, gap: 2 }}>
              <T v="bodyStrong" style={{ fontSize: 16 }}>{l.title}</T>
              <T v="small">{l.body}</T>
            </View>
          </SelectCard>
        ))}
      </View>
    </Screen>
  );
}
