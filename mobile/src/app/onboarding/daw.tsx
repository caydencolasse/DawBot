import { CircleQuestionMark } from 'lucide-react-native';
import { Image, View } from 'react-native';
import { DAWS } from '../../assets';
import { Button, CheckMark, Screen, SelectCard, StepHeader, T, Title } from '../../components/ui';
import { useStore } from '../../store';
import { colors } from '../../theme';

export default function PickDaw() {
  const { daw, setDaw } = useStore();
  return (
    <Screen scroll footer={<Button label="Continue" href="/onboarding/level" />}>
      <StepHeader value={0.25} />
      <Title title="Which DAW do you use?" sub="Lessons, menus and shortcuts will match it. You can change this anytime" style={{ paddingHorizontal: 20, paddingTop: 8 }} />
      <View accessibilityRole="radiogroup" style={{ padding: 20, paddingTop: 24, flexDirection: 'row', flexWrap: 'wrap', gap: 12 }}>
        {DAWS.map((d) => {
          const on = daw === d.id;
          return (
            <SelectCard key={d.id} selected={on} onPress={() => setDaw(d.id)} style={{ width: '47.8%', height: 64, flexDirection: 'row', alignItems: 'center', gap: 10, paddingHorizontal: 12 }}>
              {d.icon ? (
                <Image source={d.icon} style={{ width: 36, height: 36 }} accessibilityIgnoresInvertColors />
              ) : (
                <View style={{ width: 36, height: 36, borderRadius: 9, backgroundColor: colors.raised, alignItems: 'center', justifyContent: 'center' }}>
                  <CircleQuestionMark size={20} color={colors.text2} strokeWidth={1.75} />
                </View>
              )}
              <T v="body" style={{ flex: 1, fontSize: 14, lineHeight: 17 }} numberOfLines={2}>{d.name}</T>
              {on ? <CheckMark /> : null}
            </SelectCard>
          );
        })}
      </View>
    </Screen>
  );
}
