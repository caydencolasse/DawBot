import { router, useLocalSearchParams } from 'expo-router';
import { Bookmark, Ellipsis, Laptop, RefreshCw, Target } from 'lucide-react-native';
import { Pressable, Text, View } from 'react-native';
import { IDEA_ICONS, IDEA_ROUTES } from '../../components/ideas';
import { Banner, Button, IconButton, IconTile, NavHeader, Pill, Row, Screen, SectionHeader, T } from '../../components/ui';
import { useStore } from '../../store';
import { colors, fonts } from '../../theme';

const WIDE = ['Verse', 'Hook', 'Verse 2', 'Drop'];

export default function ProjectDetail() {
  const { id } = useLocalSearchParams<{ id: string }>();
  const { projects, ideas } = useStore();
  const project = projects.find((p) => p.id === id) ?? projects[0];
  const projectIdeas = ideas.filter((i) => i.projectId === project.id);
  const sketched = project.sections.filter((s) => s.sketched).length;
  const next = project.sections.find((s) => !s.sketched);

  return (
    <Screen scroll footer={<Button label="Send to studio" icon={Laptop} href={`/projects/send?id=${project.id}`} />}>
      <NavHeader right={<IconButton icon={Ellipsis} label="More options" />} />
      <View style={{ paddingHorizontal: 20, gap: 4 }}>
        <T v="title" style={{ fontSize: 28 }}>{project.name}</T>
        <T v="small" style={{ color: colors.text3 }}>{project.daw} · {project.bpm} BPM · {project.key}</T>
      </View>
      {project.status === 'synced' ? (
        <Banner icon={RefreshCw} tone="green" body="Synced to your studio · opens in the plugin" style={{ marginHorizontal: 20, marginTop: 16, paddingVertical: 12 }} />
      ) : (
        <Banner icon={Laptop} tone="neutral" body="Only on your phone so far. Send it to your studio when you're ready" style={{ marginHorizontal: 20, marginTop: 16, paddingVertical: 12 }} />
      )}

      <SectionHeader title="Song map" right={`${sketched} of ${project.sections.length} sections`} style={{ paddingHorizontal: 20, paddingTop: 24 }} />
      <Row gap={4} style={{ paddingHorizontal: 20, paddingTop: 12, alignItems: 'flex-start' }}>
        {project.sections.map((s) => (
          <View key={s.name} style={{ flex: WIDE.includes(s.name) ? 2 : 1, gap: 6 }}>
            <View style={{ height: 40, borderRadius: 8, backgroundColor: s.sketched ? colors.coral : 'transparent', borderWidth: s.sketched ? 0 : 1, borderStyle: 'dashed', borderColor: '#4A4A52' }} />
            <Text numberOfLines={1} style={{ fontFamily: fonts.regular, fontSize: 11, color: s.sketched ? colors.text : colors.text3 }}>{s.name}</Text>
          </View>
        ))}
      </Row>
      {next ? (
        <Banner icon={Target} tone="neutral" title={`Next: sketch ${next.name}`} body="Reuse the chord loop and strip the drums back to kick and hats" style={{ marginHorizontal: 20, marginTop: 16 }} />
      ) : null}

      <SectionHeader title="Ideas & notes" style={{ paddingHorizontal: 20, paddingTop: 20 }} />
      <View style={{ paddingHorizontal: 20, paddingTop: 4 }}>
        {projectIdeas.map((idea) => (
          <Pressable key={idea.id} accessibilityRole="button" onPress={() => router.push(IDEA_ROUTES[idea.kind] as never)} style={{ flexDirection: 'row', alignItems: 'center', gap: 12, paddingVertical: 12, borderBottomWidth: 1, borderBottomColor: colors.line }}>
            <IconTile icon={IDEA_ICONS[idea.kind]} />
            <View style={{ flex: 1, minWidth: 0 }}>
              <T v="bodyStrong">{idea.title}</T>
              <T v="muted" numberOfLines={1}>{idea.meta}</T>
            </View>
            {idea.section ? <Pill label={idea.section} /> : null}
          </Pressable>
        ))}
        {project.id === 'night-drive' ? (
          <Pressable accessibilityRole="button" onPress={() => router.push('/projects/saved' as never)} style={{ flexDirection: 'row', alignItems: 'center', gap: 12, paddingVertical: 12 }}>
            <IconTile icon={Bookmark} />
            <View style={{ flex: 1 }}>
              <T v="bodyStrong">Vocal stutter recipe</T>
              <T v="muted">Saved from the plugin</T>
            </View>
            <Pill label="Hook" />
          </Pressable>
        ) : null}
      </View>
    </Screen>
  );
}
