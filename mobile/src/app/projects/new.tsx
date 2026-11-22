import { router } from 'expo-router';
import { ChevronRight } from 'lucide-react-native';
import { useState } from 'react';
import { Pressable, Text, TextInput, View } from 'react-native';
import { Button, DawIcon, NavHeader, Row, Screen, SelectCard, T } from '../../components/ui';
import { SONG_SECTIONS, useStore } from '../../store';
import { colors, fonts, radius } from '../../theme';

const TEMPLATES = [
  { id: 'blank', title: 'Blank', body: 'Start from nothing', sections: ['Idea'] },
  { id: 'song', title: 'Song', body: 'Intro, verses, hooks, bridge', sections: SONG_SECTIONS },
  { id: 'beat', title: 'Beat', body: 'Loop-based, 8 sections', sections: ['A1', 'A2', 'B1', 'B2', 'A3', 'C', 'A4', 'Outro'] },
  { id: 'club', title: 'Club track', body: 'Build, drop, breakdown', sections: ['Intro', 'Build', 'Drop', 'Break', 'Drop 2', 'Outro'] },
];

function Field({ label, value, onChangeText, id }: { label: string; value: string; onChangeText: (v: string) => void; id: string }) {
  return (
    <View style={{ gap: 8, flex: 1 }}>
      <T v="small" style={{ fontSize: 13 }}>{label}</T>
      <TextInput
        nativeID={id}
        accessibilityLabel={label}
        value={value}
        onChangeText={onChangeText}
        placeholderTextColor={colors.text3}
        style={{ height: 48, borderRadius: radius.md, borderWidth: 1, borderColor: 'rgba(255,255,255,0.1)', backgroundColor: colors.card, color: colors.text, paddingHorizontal: 14, fontFamily: fonts.regular, fontSize: 15 }}
      />
    </View>
  );
}

export default function NewProject() {
  const { addProject } = useStore();
  const [name, setName] = useState('Night Drive 2');
  const [bpm, setBpm] = useState('92');
  const [key, setKey] = useState('F minor');
  const [vibe, setVibe] = useState('Late-night synthwave, warm bass');
  const [template, setTemplate] = useState('song');

  const create = () => {
    const t = TEMPLATES.find((x) => x.id === template) ?? TEMPLATES[1];
    const id = addProject({
      name: name.trim() || 'Untitled',
      daw: 'Ableton Live 12',
      bpm: parseInt(bpm, 10) || 120,
      key: key.trim() || 'C major',
      sections: t.sections.map((s) => ({ name: s, sketched: false })),
    });
    router.replace(`/projects/${id}` as never);
  };

  return (
    <Screen scroll edges={['top', 'bottom']} footer={<Button label="Create project" onPress={create} disabled={!name.trim()} />}>
      <NavHeader title="New project" close />
      <View style={{ paddingHorizontal: 20, gap: 16 }}>
        <Field id="name" label="Project name" value={name} onChangeText={setName} />
        <View style={{ gap: 8 }}>
          <T v="small" style={{ fontSize: 13 }}>DAW</T>
          <Pressable accessibilityRole="button" accessibilityLabel="DAW, Ableton Live 12" style={{ height: 48, borderRadius: radius.md, borderWidth: 1, borderColor: 'rgba(255,255,255,0.1)', backgroundColor: colors.card, paddingHorizontal: 14, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' }}>
            <Row gap={10}>
              <DawIcon id="ableton" size={24} />
              <Text style={{ fontFamily: fonts.regular, fontSize: 15, color: colors.text }}>Ableton Live 12</Text>
            </Row>
            <ChevronRight size={16} color={colors.text3} />
          </Pressable>
        </View>
        <Row gap={12}>
          <Field id="bpm" label="Tempo (BPM)" value={bpm} onChangeText={setBpm} />
          <Field id="key" label="Key" value={key} onChangeText={setKey} />
        </Row>
        <Field id="vibe" label="Vibe or reference (optional)" value={vibe} onChangeText={setVibe} />
        <View style={{ gap: 8 }}>
          <T v="small" style={{ fontSize: 13 }}>Song map</T>
          <View accessibilityRole="radiogroup" style={{ flexDirection: 'row', flexWrap: 'wrap', gap: 8 }}>
            {TEMPLATES.map((t) => (
              <SelectCard key={t.id} selected={template === t.id} onPress={() => setTemplate(t.id)} style={{ width: '48.5%', padding: 14, gap: 4 }}>
                <T v="bodyStrong">{t.title}</T>
                <T v="muted">{t.body}</T>
              </SelectCard>
            ))}
          </View>
        </View>
      </View>
    </Screen>
  );
}
