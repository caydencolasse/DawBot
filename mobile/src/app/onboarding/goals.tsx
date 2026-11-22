import { AudioLines, Drum, Laptop, Layers, Mic, Music, Piano, SlidersHorizontal, type LucideIcon } from 'lucide-react-native';
import { View } from 'react-native';
import { Button, Chip, Screen, SelectCard, StepHeader, T, Title } from '../../components/ui';
import { useStore, type Topic } from '../../store';
import { colors } from '../../theme';

const TOPICS: { id: Topic; label: string; icon: LucideIcon }[] = [
  { id: 'beats', label: 'Beats & drums', icon: Drum },
  { id: 'chords', label: 'Chords & melody', icon: Piano },
  { id: 'vocals', label: 'Vocals', icon: Mic },
  { id: 'sound', label: 'Sound design', icon: AudioLines },
  { id: 'mixing', label: 'Mixing', icon: SlidersHorizontal },
  { id: 'arrangement', label: 'Arrangement', icon: Layers },
  { id: 'theory', label: 'Music theory', icon: Music },
  { id: 'daw', label: 'Your DAW', icon: Laptop },
];

const GOALS = [
  { minutes: 5, label: 'Casual' },
  { minutes: 10, label: 'Regular' },
  { minutes: 15, label: 'Serious' },
  { minutes: 20, label: 'Intense' },
];

export default function PickGoals() {
  const { topics, toggleTopic, dailyMinutes, setDailyMinutes } = useStore();
  return (
    <Screen scroll footer={<Button label="Continue" href="/onboarding/account" disabled={topics.length === 0} />}>
      <StepHeader value={0.75} />
      <Title title="What do you want to get better at?" sub="Pick as many as you like" style={{ paddingHorizontal: 20, paddingTop: 8 }} />
      <View style={{ paddingHorizontal: 20, paddingTop: 20, flexDirection: 'row', flexWrap: 'wrap', gap: 8 }}>
        {TOPICS.map((t) => (
          <Chip key={t.id} label={t.label} icon={t.icon} on={topics.includes(t.id)} onPress={() => toggleTopic(t.id)} />
        ))}
      </View>
      <View style={{ paddingHorizontal: 20, paddingTop: 32, gap: 12 }}>
        <T v="section">Daily goal</T>
        <View accessibilityRole="radiogroup" style={{ gap: 8 }}>
          {GOALS.map((g) => {
            const on = dailyMinutes === g.minutes;
            return (
              <SelectCard key={g.minutes} selected={on} onPress={() => setDailyMinutes(g.minutes)} style={{ height: 52, paddingHorizontal: 16, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' }}>
                <T v="body" style={{ fontFamily: 'Inter_500Medium' }}>{g.minutes} min a day</T>
                <T v="small" style={{ color: on ? colors.coralText : colors.text3 }}>{g.label}</T>
              </SelectCard>
            );
          })}
        </View>
      </View>
    </Screen>
  );
}
