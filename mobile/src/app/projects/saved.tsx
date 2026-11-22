import { BookOpen, Bookmark, Ellipsis, Laptop } from 'lucide-react-native';
import { Text, View } from 'react-native';
import { Button, Card, IconButton, NavHeader, Pill, Row, Screen, T } from '../../components/ui';
import { colors, fonts } from '../../theme';

// An answer saved in the plugin (see the plugin's "Save to a project"), readable on the phone.
const STEPS = [
  { title: 'Choose a vocal phrase', body: 'Pick a short line with a clear beginning' },
  { title: 'Slice it into Simpler', body: 'Set Slice by Transient, then play slices from a MIDI clip' },
  { title: 'Repeat with variation', body: 'Add Beat Repeat at 1/16 with a 30% chance' },
];

export default function SavedAnswer() {
  return (
    <Screen
      scroll
      footer={
        <>
          <Button label="Practice it as a lesson" variant="secondary" icon={BookOpen} href="/lesson/start" />
          <Button label="Do it in my DAW" icon={Laptop} href="/projects/send?id=night-drive" />
        </>
      }
    >
      <NavHeader right={<IconButton icon={Ellipsis} label="More options" />} />
      <View style={{ paddingHorizontal: 20, gap: 8 }}>
        <Row gap={6}>
          <Bookmark size={14} color={colors.coralText} strokeWidth={2} />
          <T v="eyebrow">Saved from the plugin · Ableton Live 12</T>
        </Row>
        <T v="title">Vocal stutter recipe</T>
        <T v="sub">A short vocal slice with rhythmic variation, in the style of Fred again..</T>
      </View>
      <View style={{ paddingHorizontal: 20, paddingTop: 24, gap: 10 }}>
        {STEPS.map((s, i) => (
          <Card key={s.title} style={{ paddingHorizontal: 16, paddingVertical: 14, flexDirection: 'row', gap: 12 }}>
            <View style={{ width: 24, height: 24, borderRadius: 12, backgroundColor: colors.coral, alignItems: 'center', justifyContent: 'center' }}>
              <Text style={{ fontFamily: fonts.semibold, fontSize: 13, color: colors.onCoral }}>{i + 1}</Text>
            </View>
            <View style={{ flex: 1 }}>
              <T v="bodyStrong">{s.title}</T>
              <T v="small" style={{ marginTop: 2 }}>{s.body}</T>
            </View>
          </Card>
        ))}
      </View>
      <Row gap={8} style={{ paddingHorizontal: 20, paddingTop: 16, flexWrap: 'wrap' }}>
        <Pill label="Simpler" />
        <Pill label="Beat Repeat" />
        <Pill label="Stock plugins only" />
      </Row>
    </Screen>
  );
}
