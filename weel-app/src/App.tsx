import React from 'react';
import NodeVisualization from './components/NodeVisualization';
import './App.css';

const App: React.FC = () => {
  return (
    <div className="App" style={{ width: '100vw', height: '100vh' }}>
      <NodeVisualization />
    </div>
  );
};

export default App;