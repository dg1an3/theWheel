import React, { useState } from 'react';
import NodeVisualization from './components/NodeVisualization';
import { ElliptangleEditor } from './components/ElliptangleEditor';
import './App.css';

type ViewMode = '3d' | '2d';

const App: React.FC = () => {
  const [mode, setMode] = useState<ViewMode>('3d');

  return (
    <div className="App" style={{ width: '100vw', height: '100vh', display: 'flex', flexDirection: 'column' }}>
      <div className="tab-bar">
        <button
          className={`tab-button ${mode === '3d' ? 'active' : ''}`}
          onClick={() => setMode('3d')}
        >
          3D Node Graph
        </button>
        <button
          className={`tab-button ${mode === '2d' ? 'active' : ''}`}
          onClick={() => setMode('2d')}
        >
          Elliptangle Editor
        </button>
      </div>
      <div style={{ flex: 1, overflow: 'hidden' }}>
        {mode === '3d' ? <NodeVisualization /> : <ElliptangleEditor />}
      </div>
    </div>
  );
};

export default App;
