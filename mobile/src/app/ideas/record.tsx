import { router } from 'expo-router';
import { Headphones, Square } from 'lucide-react-native';
import { useEffect, useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { Card, NavHeader, Row, Screen, T, Toggle, Waveform } from '../../components/ui';
import { colors, fonts } from '../../theme';

// TODO(audio): record with expo-audio, stream to the hum-to-MIDI endpoint
// (e.g. Spotify's Basic Pitch on the server) and show live key/tempo estimates.
export default function Record() {
  const [seconds, setSeconds] = useState(0);
  const [click, setClick] = useState(true);

  useEffect(() => {
    const t = setInterval(() => setSeconds((s) => s + 1), 1000);
    return () => clearInterval(t);
  }, []);

  const played = Math.min(48, 6 + seconds * 3);

  return (
    <Screen>
      <NavHeader title="New idea" close />
      <View style={{ alignItems: 'center', gap: 6, paddingTop: 8 }}>
        <Row gap={8}>
          <View style={{ width: 8, height: 8, borderRadius: 4, backgroundColor: colors.coral }} />
          <Text style={{ fontFamily: fonts.semibold, fontSize: 14, color: colors.coralText }}>Recording</Text>
        </Row>
        <Text accessibilityLiveRegion="polite" style={{ fontFamily: fonts.semibold, fontSize: 48, color: colors.text, fontVariant: ['tabular-nums'], letterSpacing: -1 }}>
          {Math.floor(seconds / 60)}:{String(seconds % 60).padStart(2, '0')}
        </Text>
      </View>
      <View style={{ paddingHorizontal: 20, marginTop: 32, height: 140, justifyContent: 'center' }}>
        <Waveform bars={48} seed={11} played={played} height={140} barWidth={4} />
      </View>
      <Row gap={10} style={{ paddingHorizontal: 20, marginTop: 32 }}>
        <Card style={{ flex: 1, paddingHorizontal: 16, paddingVertical: 14 }}>
          <T v="muted">Key, so far</T>
          <T v="bodyStrong" style={{ fontSize: 18, marginTop: 4 }}>F minor</T>
        </Card>
        <Card style={{ flex: 1, paddingHorizontal: 16, paddingVertical: 14 }}>
          <T v="muted">Tempo</T>
          <T v="bodyStrong" style={{ fontSize: 18, marginTop: 4 }}>92 BPM</T>
        </Card>
      </Row>
      <Card style={{ marginHorizontal: 20, marginTop: 12, height: 52, paddingHorizontal: 16, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' }}>
        <Row gap={10}>
          <Headphones size={20} color={colors.text2} strokeWidth={1.75} />
          <T>Click track</T>
        </Row>
        <Toggle value={click} onChange={setClick} label="Click track" />
      </Card>
      <T v="sub" style={{ textAlign: 'center', paddingHorizontal: 32, paddingTop: 20 }}>Hum, sing, whistle or beatbox. dawbot turns it into MIDI</T>
      <View style={{ flex: 1, justifyContent: 'flex-end', alignItems: 'center', paddingBottom: 48 }}>
        <Pressable
          accessibilityRole="button"
          accessibilityLabel="Stop recording"
          onPress={() => router.replace('/ideas/hum' as never)}
          style={{ width: 80, height: 80, borderRadius: 40, borderWidth: 4, borderColor: 'rgba(244,97,63,0.35)', alignItems: 'center', justifyContent: 'center' }}
        >
          <View style={{ width: 64, height: 64, borderRadius: 32, backgroundColor: colors.coral, alignItems: 'center', justifyContent: 'center' }}>
            <Square size={26} color={colors.onCoral} fill={colors.onCoral} />
          </View>
        </Pressable>
      </View>
    </Screen>
  );
}
