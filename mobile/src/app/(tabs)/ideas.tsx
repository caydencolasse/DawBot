import { router } from 'expo-router';
import { LayoutGrid, Mic, PenLine, Piano, Search, type LucideIcon } from 'lucide-react-native';
import { Pressable, Text, View } from 'react-native';
import { IDEA_ICONS, IDEA_ROUTES } from '../../components/ideas';
import { Card, IconButton, IconTile, Pill, Row, Screen, SectionHeader, T, Waveform } from '../../components/ui';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

const CAPTURE: { label: string; icon: LucideIcon; href: string; primary?: boolean }[] = [
  { label: 'Hum', icon: Mic, href: '/ideas/record', primary: true },
  { label: 'Chords', icon: Piano, href: '/ideas/chords' },
  { label: 'Beat', icon: LayoutGrid, href: '/ideas/beat' },
  { label: 'Note', icon: PenLine, href: '/ideas/record' },
];

export default function Ideas() {
  const { ideas, projects } = useStore();
  const projectName = (id: string | null) => projects.find((p) => p.id === id)?.name ?? 'Unsorted';

  return (
    <Screen scroll>
      <Row style={{ paddingHorizontal: 20, paddingTop: 8, justifyContent: 'space-between' }}>
        <T v="display">Ideas</T>
        <IconButton icon={Search} label="Search ideas" tone="card" size={20} />
      </Row>
      <T v="sub" style={{ paddingHorizontal: 20, paddingTop: 4 }}>Catch it now, finish it in the studio</T>

      <Row gap={8} style={{ paddingHorizontal: 20, paddingTop: 20 }}>
        {CAPTURE.map((c) => (
          <Pressable
            key={c.label}
            accessibilityRole="button"
            accessibilityLabel={`New ${c.label.toLowerCase()} idea`}
            onPress={() => router.push(c.href as never)}
            style={({ pressed }) => ({ flex: 1, height: 88, borderRadius: 16, alignItems: 'center', justifyContent: 'center', gap: 8, backgroundColor: c.primary ? colors.coral : colors.card, borderWidth: c.primary ? 0 : 1, borderColor: colors.line, opacity: pressed ? 0.85 : 1 })}
          >
            <c.icon size={24} color={c.primary ? colors.onCoral : colors.text} strokeWidth={1.75} />
            <Text style={{ fontFamily: fonts.semibold, fontSize: 13, color: c.primary ? colors.onCoral : colors.text }}>{c.label}</Text>
          </Pressable>
        ))}
      </Row>

      <SectionHeader title="Recent" right="See all" style={{ paddingHorizontal: 20, paddingTop: 28, paddingBottom: 12 }} />
      <View style={{ paddingHorizontal: 20, gap: 10 }}>
        {ideas.map((idea) => (
          <Card key={idea.id} onPress={() => router.push(IDEA_ROUTES[idea.kind] as never)} style={{ paddingHorizontal: 16, paddingVertical: 14, gap: 10 }}>
            <Row>
              <IconTile icon={IDEA_ICONS[idea.kind]} />
              <View style={{ flex: 1, minWidth: 0 }}>
                <T v="bodyStrong">{idea.title}</T>
                <T v="muted" numberOfLines={1}>{idea.meta}</T>
              </View>
              <Pill label={projectName(idea.projectId)} tone={idea.projectId ? 'neutral' : 'muted'} />
            </Row>
            {idea.kind === 'hum' ? <Waveform bars={46} seed={2} played={20} height={28} gap={2} /> : null}
          </Card>
        ))}
      </View>
    </Screen>
  );
}
