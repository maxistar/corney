import React, { useMemo, useState } from 'react';
import { StlViewer } from 'react-stl-viewer';

const viewerStyle = {
  width: '100%',
  height: 'min(64vh, 620px)',
  minHeight: '360px',
  background: '#111b21',
  borderRadius: '10px',
  border: '1px solid #34454b',
  overflow: 'hidden',
};

export default function STLModelBrowser({ groups }) {
  const safeGroups = Array.isArray(groups) ? groups : [];
  const [groupIndex, setGroupIndex] = useState(0);
  const [modelIndex, setModelIndex] = useState(0);
  const group = safeGroups[groupIndex] ?? safeGroups[0];
  const models = group?.models ?? [];
  const model = models[modelIndex] ?? models[0];
  const stats = useMemo(() => {
    const count = safeGroups.reduce((total, item) => total + (item.models?.length ?? 0), 0);
    return `${count} STL files`;
  }, [safeGroups]);

  if (!group || !model) {
    return <p className="model-empty">No printable models are available.</p>;
  }

  function selectGroup(index) {
    setGroupIndex(index);
    setModelIndex(0);
  }

  return (
    <div className="model-browser">
      <div className="model-sidebar" aria-label="Model groups">
        <div className="model-sidebar-head">
          <span>Model sets</span>
          <strong>{stats}</strong>
        </div>
        {safeGroups.map((item, index) => (
          <button
            className={`model-group ${index === groupIndex ? 'is-active' : ''}`}
            key={item.id}
            type="button"
            onClick={() => selectGroup(index)}
          >
            <span>{item.name}</span>
            <small>{item.models.length} files</small>
          </button>
        ))}
      </div>

      <div className="model-workspace">
        <div className="model-viewer-wrap">
          <StlViewer
            url={model.url}
            style={viewerStyle}
            orbitControls
            shadows
            showAxes
            modelProps={{ color: model.color ?? '#d9f488', scale: 1 }}
            cameraProps={{
              initialPosition: {
                latitude: 0.65,
                longitude: 0.8,
                distance: model.distance ?? 2.8,
              },
            }}
          />
        </div>

        <div className="model-detail">
          <div>
            <span className="model-kicker">{group.name}</span>
            <h2>{model.name}</h2>
            <p>{model.description}</p>
          </div>
          <div className="model-actions">
            <a className="btn" href={model.url} download>
              Download .STL
            </a>
            <a className="btn btn-outline" href={model.source}>
              View source
            </a>
          </div>
        </div>

        <div className="model-strip" aria-label={`${group.name} models`}>
          {models.map((item, index) => (
            <button
              className={`model-item ${index === modelIndex ? 'is-active' : ''}`}
              key={item.url}
              type="button"
              onClick={() => setModelIndex(index)}
            >
              <span>{item.name}</span>
              <small>{item.filename}</small>
            </button>
          ))}
        </div>
      </div>
    </div>
  );
}
