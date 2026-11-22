import { router } from 'expo-router';
import { ArrowUp, Laptop, Mic, Play, Plus } from 'lucide-react-native';
import { useRef, useState } from 'react';
import { Image, KeyboardAvoidingView, Platform, Pressable, ScrollView, Text, TextInput, View, type ScrollViewInstance } from 'react-native';
import { images } from '../../assets';
import { Banner, Card, Chip, DawPill, Row, T } from '../../components/ui';
import { colors, fonts } from '../../theme';
import { SafeAreaView } from 'react-native-safe-area-context';

type Message =
  | { role: 'user'; text: string }
  | { role: 'bot'; text: string; chords?: string[]; chordMeta?: string };

const START: Message[] = [
  { role: 'user', text: 'Why does a minor progression sound sad?' },
  {
    role: 'bot',
    text: 'The minor third is the key. In A minor, the A to C gap is a half step smaller than A to C♯ in A major, and our ears hear that darker, unresolved color',
    chords: ['Am', 'F', 'C', 'G'],
    chordMeta: 'A minor · 90 BPM',
  },
];

const SUGGESTIONS = ['Make it happier', "What's a relative major?"];

// The phone answers theory, sound and technique questions. Hands-on DAW help lives in the plugin.
// TODO(api): send messages to the dawbot proxy (server/coach.mjs) with mode "mobile".
export default function Ask() {
  const [messages, setMessages] = useState<Message[]>(START);
  const [draft, setDraft] = useState('');
  const scroll = useRef<ScrollViewInstance>(null);

  const send = (text: string) => {
    const t = text.trim();
    if (!t) return;
    setMessages((m) => [...m, { role: 'user', text: t }, { role: 'bot', text: 'Good question. Here is a quick way to hear it: play the same loop and swap one chord at a time.' }]);
    setDraft('');
    setTimeout(() => scroll.current?.scrollToEnd({ animated: true }), 50);
  };

  return (
    <SafeAreaView style={{ flex: 1, backgroundColor: colors.bg }} edges={['top']}>
      <KeyboardAvoidingView style={{ flex: 1 }} behavior={Platform.OS === 'ios' ? 'padding' : undefined}>
        <Row style={{ paddingHorizontal: 20, paddingTop: 8, justifyContent: 'space-between' }}>
          <T v="display">Ask</T>
          <DawPill />
        </Row>
        <Banner icon={Laptop} tone="neutral" body="For hands-on help inside your DAW, ask the plugin" style={{ marginHorizontal: 20, marginTop: 12, paddingVertical: 10 }} />
        <ScrollView ref={scroll} contentContainerStyle={{ padding: 20, gap: 16 }}>
          {messages.map((m, i) =>
            m.role === 'user' ? (
              <View key={i} style={{ alignSelf: 'flex-end', maxWidth: '80%', paddingHorizontal: 14, paddingVertical: 12, borderRadius: 16, borderBottomRightRadius: 4, backgroundColor: colors.raised }}>
                <T>{m.text}</T>
              </View>
            ) : (
              <View key={i} style={{ flexDirection: 'row', gap: 10, alignItems: 'flex-start' }}>
                <Image source={images.logo} style={{ width: 28, height: 28, marginTop: 2 }} accessibilityIgnoresInvertColors />
                <View style={{ flex: 1, gap: 12 }}>
                  <T style={{ lineHeight: 23 }}>{m.text}</T>
                  {m.chords ? (
                    <Card style={{ padding: 12, gap: 10 }}>
                      <Row style={{ justifyContent: 'space-between' }}>
                        <T v="bodyStrong" style={{ fontSize: 14 }}>Try this loop</T>
                        <T v="muted">{m.chordMeta}</T>
                      </Row>
                      <Row gap={6}>
                        {m.chords.map((c) => (
                          <View key={c} style={{ flex: 1, height: 40, borderRadius: 8, backgroundColor: colors.raised, alignItems: 'center', justifyContent: 'center' }}>
                            <Text style={{ fontFamily: fonts.semibold, fontSize: 15, color: colors.text }}>{c}</Text>
                          </View>
                        ))}
                      </Row>
                      <Row gap={8}>
                        <Chip small label="Play" icon={Play} />
                        <Chip small label="Save as idea" icon={Plus} onPress={() => router.push('/ideas/chords' as never)} />
                      </Row>
                    </Card>
                  ) : null}
                </View>
              </View>
            ),
          )}
          <Row gap={8} style={{ flexWrap: 'wrap' }}>
            {SUGGESTIONS.map((s) => (
              <Chip key={s} small label={s} onPress={() => send(s)} />
            ))}
          </Row>
        </ScrollView>
        <View style={{ paddingHorizontal: 20, paddingVertical: 12 }}>
          <View style={{ height: 52, paddingLeft: 16, paddingRight: 6, borderRadius: 16, backgroundColor: colors.card, borderWidth: 1, borderColor: 'rgba(255,255,255,0.1)', flexDirection: 'row', alignItems: 'center', gap: 8 }}>
            <TextInput
              accessibilityLabel="Ask dawbot"
              placeholder="Ask about theory, sounds or your DAW"
              placeholderTextColor={colors.text3}
              value={draft}
              onChangeText={setDraft}
              onSubmitEditing={() => send(draft)}
              returnKeyType="send"
              style={{ flex: 1, fontFamily: fonts.regular, fontSize: 15, color: colors.text }}
            />
            <Pressable accessibilityRole="button" accessibilityLabel="Record a question" style={{ width: 40, height: 40, alignItems: 'center', justifyContent: 'center' }}>
              <Mic size={20} color={colors.text2} strokeWidth={1.75} />
            </Pressable>
            <Pressable accessibilityRole="button" accessibilityLabel="Send" onPress={() => send(draft)} style={{ width: 40, height: 40, borderRadius: 12, backgroundColor: colors.coral, alignItems: 'center', justifyContent: 'center' }}>
              <ArrowUp size={20} color={colors.onCoral} strokeWidth={2} />
            </Pressable>
          </View>
        </View>
      </KeyboardAvoidingView>
    </SafeAreaView>
  );
}
