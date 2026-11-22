import { router, useLocalSearchParams } from 'expo-router';
import { Bookmark, Check, Laptop, Music, SlidersHorizontal, Target, type LucideIcon } from 'lucide-react-native';
import { useEffect, useState } from 'react';
import { View } from 'react-native';
import { Button, Card, NavHeader, Row, Screen, T, Toggle } from '../../components/ui';
import { CURRENT_LESSON } from '../../data/lessons';
import { useStore } from '../../store';
import { colors } from '../../theme';

// Handoff to the dawbot plugin. Used for projects (?id=) and lessons (?lesson=).
// TODO(sync): POST to the dawbot API so the plugin picks this up on next launch.
export default function SendToStudio() {
  const { id, lesson } = useLocalSearchParams<{ id?: string; lesson?: string }>();
  const { projects, ideas, sendToStudio } = useStore();
  const project = projects.find((p) => p.id === id);
  const [notify, setNotify] = useState(true);

  useEffect(() => {
    if (project) sendToStudio(project.id);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [project?.id]);

  const isLesson = !!lesson && !project;
  const name = isLesson ? CURRENT_LESSON.title : (project?.name ?? 'Night Drive');
  const clipCount = ideas.filter((i) => i.projectId === (project?.id ?? 'night-drive') && i.kind !== 'note').length;

  const items: { icon: LucideIcon; title: string; body: string }[] = isLesson
    ? [
        { icon: Target, title: 'Guided walkthrough', body: 'Build the i, iv, VI and VII chords on a real track' },
        { icon: Music, title: 'MIDI starter clip', body: 'E minor chords, ready to drag in' },
      ]
    : [
        { icon: Music, title: `${clipCount} MIDI ${clipCount === 1 ? 'clip' : 'clips'}`, body: 'Hums, chord loops and beats' },
        { icon: SlidersHorizontal, title: 'Tempo and key', body: `${project?.bpm ?? 92} BPM · ${project?.key ?? 'F minor'}` },
        { icon: Target, title: 'Song map', body: `${project?.sections.length ?? 6} sections` },
        { icon: Bookmark, title: 'Notes', body: 'Including saved answers from the plugin' },
      ];

  return (
    <Screen scroll edges={['top', 'bottom']} footer={<Button label="Done" onPress={() => router.back()} />}>
      <NavHeader close />
      <View style={{ alignItems: 'center', gap: 12, paddingHorizontal: 20 }}>
        <View style={{ width: 72, height: 72, borderRadius: 22, backgroundColor: colors.coralBg, alignItems: 'center', justifyContent: 'center' }}>
          <Laptop size={34} color={colors.coral} strokeWidth={1.75} />
        </View>
        <T v="title" style={{ textAlign: 'center' }}>{name} is ready in your studio</T>
        <T v="sub" style={{ textAlign: 'center' }}>It opens in the dawbot plugin the next time you launch Ableton Live 12</T>
      </View>
      <Card style={{ marginHorizontal: 20, marginTop: 20, paddingHorizontal: 14, paddingVertical: 12, flexDirection: 'row', alignItems: 'center', gap: 10 }}>
        <View style={{ width: 8, height: 8, borderRadius: 4, backgroundColor: colors.green }} />
        <T style={{ flex: 1, fontSize: 14 }}>MacBook Pro · plugin online</T>
        <T v="muted">2 min ago</T>
      </Card>
      <View style={{ paddingHorizontal: 20, paddingTop: 12 }}>
        {items.map((it) => (
          <Row key={it.title} style={{ paddingVertical: 12, borderBottomWidth: 1, borderBottomColor: colors.line }}>
            <it.icon size={20} color={colors.coralText} strokeWidth={1.75} />
            <View style={{ flex: 1 }}>
              <T v="bodyStrong">{it.title}</T>
              <T v="muted">{it.body}</T>
            </View>
            <Check size={18} color={colors.green} strokeWidth={2.25} />
          </Row>
        ))}
      </View>
      <Row style={{ paddingHorizontal: 20, paddingTop: 12, height: 60, justifyContent: 'space-between' }}>
        <T>Notify me when it’s opened</T>
        <Toggle value={notify} onChange={setNotify} label="Notify me when it's opened" />
      </Row>
    </Screen>
  );
}
