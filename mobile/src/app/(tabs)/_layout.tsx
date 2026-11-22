import { Tabs } from 'expo-router';
import { BookOpen, Folder, MessageCircle, Mic, User, type LucideIcon } from 'lucide-react-native';
import { colors, fonts } from '../../theme';

const icon = (Icon: LucideIcon) =>
  function TabIcon({ color }: { color: unknown }) {
    return <Icon size={24} color={typeof color === 'string' ? color : colors.text3} strokeWidth={1.75} />;
  };

export default function TabsLayout() {
  return (
    <Tabs
      screenOptions={{
        headerShown: false,
        tabBarActiveTintColor: colors.coral,
        tabBarInactiveTintColor: colors.text3,
        tabBarLabelStyle: { fontFamily: fonts.medium, fontSize: 11 },
        tabBarStyle: { backgroundColor: 'rgba(22,22,24,0.97)', borderTopColor: colors.line },
        sceneStyle: { backgroundColor: colors.bg },
      }}
    >
      <Tabs.Screen name="learn" options={{ title: 'Learn', tabBarIcon: icon(BookOpen) }} />
      <Tabs.Screen name="ideas" options={{ title: 'Ideas', tabBarIcon: icon(Mic) }} />
      <Tabs.Screen name="projects" options={{ title: 'Projects', tabBarIcon: icon(Folder) }} />
      <Tabs.Screen name="ask" options={{ title: 'Ask', tabBarIcon: icon(MessageCircle) }} />
      <Tabs.Screen name="profile" options={{ title: 'Profile', tabBarIcon: icon(User) }} />
    </Tabs>
  );
}
