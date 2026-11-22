import { router } from 'expo-router';
import { Download } from 'lucide-react-native';
import { useEffect, useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { Banner, Button, Card, NavHeader, Row, Screen, T, Title } from '../../components/ui';
import { colors, fonts } from '../../theme';

const STEPS = ['Open dawbot in your DAW', 'Go to Settings, then Link phone', 'Enter this code'];

// TODO(sync): request a real pairing code from the dawbot API and poll until the plugin confirms.
export default function LinkStudio() {
  const [secondsLeft, setSecondsLeft] = useState(582);
  const [emailed, setEmailed] = useState(false);

  useEffect(() => {
    const t = setInterval(() => setSecondsLeft((s) => Math.max(0, s - 1)), 1000);
    return () => clearInterval(t);
  }, []);

  const code = '482913';
  const mm = Math.floor(secondsLeft / 60);
  const ss = String(secondsLeft % 60).padStart(2, '0');

  return (
    <Screen
      scroll
      footer={
        <>
          <Button label="Start learning" onPress={() => router.replace('/learn' as never)} />
          <Button label="Skip for now" variant="ghost" onPress={() => router.replace('/learn' as never)} />
        </>
      }
    >
      <NavHeader />
      <Title title="Link your studio" sub="Pair the plugin once and everything you make here shows up in your DAW" style={{ paddingHorizontal: 20 }} />
      <Card style={{ margin: 20, marginTop: 24, padding: 20, gap: 16 }}>
        {STEPS.map((s, i) => (
          <Row key={s}>
            <View style={{ width: 24, height: 24, borderRadius: 12, backgroundColor: colors.raised, alignItems: 'center', justifyContent: 'center' }}>
              <Text style={{ fontFamily: fonts.semibold, fontSize: 13, color: colors.text }}>{i + 1}</Text>
            </View>
            <T>{s}</T>
          </Row>
        ))}
        <View accessibilityLabel={`Pairing code ${code.slice(0, 3)} ${code.slice(3)}`} style={{ flexDirection: 'row', justifyContent: 'center', gap: 8, paddingVertical: 8 }}>
          {code.split('').map((d, i) => (
            <View key={i} style={{ width: 44, height: 56, borderRadius: 12, backgroundColor: colors.raised, borderWidth: 1, borderColor: colors.line, alignItems: 'center', justifyContent: 'center', marginLeft: i === 3 ? 8 : 0 }}>
              <Text style={{ fontFamily: fonts.semibold, fontSize: 26, color: colors.text, fontVariant: ['tabular-nums'] }}>{d}</Text>
            </View>
          ))}
        </View>
        <T v="muted" style={{ textAlign: 'center' }}>Code expires in {mm}:{ss}</T>
      </Card>
      <Banner icon={Download} title="No plugin yet?" body="We'll email you the VST3 / AU download for macOS or Windows" style={{ marginHorizontal: 20 }}>
        <Pressable accessibilityRole="button" onPress={() => setEmailed(true)} style={{ marginTop: 6 }}>
          <Text style={{ fontFamily: fonts.semibold, fontSize: 14, color: colors.coralText }}>{emailed ? 'Sent. Check your inbox' : 'Email me the link'}</Text>
        </Pressable>
      </Banner>
    </Screen>
  );
}
